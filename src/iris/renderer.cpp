#include "renderer.hpp"
#include "iris/data/DataObject.hpp"
#include <cassert>
#include <numeric>
#include <cmath>
#include "iris/exceptions.hpp"
#include "iris/utils.hpp"
#include "throw.hpp"

namespace iris {

void Renderer::visit(const ExtendsStatementNode &node) {
    const auto included_template_it = template_storage.find(node.file);
    if (included_template_it != template_storage.end()) {
        const Template *parent_template = &included_template_it->second;
        render_to(
            *output_stream,
            *parent_template,
            appendSourceLocStack(&node),
            *data_input,
            &additional_data
        );
        break_rendering = true;
    } else if (config.throw_at_missing_includes) {
        throw_renderer_error("extends '" + node.file + "' not found", node);
    }
}
void Renderer::visit(const BlockStatementNode &node) {
    const size_t old_level = current_level;
    current_level = 0;
    current_template = template_stack.front();
    const auto block_it = current_template->block_storage.find(node.name);
    if (block_it != current_template->block_storage.end()) {
        block_statement_stack.emplace_back(&node);
        block_it->second->block.accept(*this);
        block_statement_stack.pop_back();
    }
    current_level = old_level;
    current_template = template_stack.back();
}
void Renderer::visit(const SetStatementNode &node) {
    DataPointer ptr(node.key);
    additional_data[ptr] = *eval_expression_list(node.expression);
}
void Renderer::visit(const IncludeStatementNode &node) {
    auto sub_renderer = Renderer(
        config,
        template_storage,
        function_storage,
        udata
    );
    const auto included_template_it = template_storage.find(node.file);
    if (included_template_it != template_storage.end()) {
        sub_renderer.render_to(
            *output_stream,
            included_template_it->second,
            appendSourceLocStack(&node),
            *data_input,
            &additional_data
        );
    } else if (config.throw_at_missing_includes) {
        throw_renderer_error("include '" + node.file + "' not found", node);
    }
}
Renderer::Renderer(
    const RenderConfig &config,
    const TemplateStorage &template_storage,
    const FunctionStorage &function_storage,
    void* udata
) : udata(udata),
    config(config),
    template_storage(template_storage),
    function_storage(function_storage) {}

void Renderer::render_to(
    std::ostream &os,
    const Template &tmpl,
    const std::stack<SourceLocation>& importStack,
    const DataObject &data,
    DataObject *loop_data
) {
    output_stream = &os;
    current_template = &tmpl;
    this->importStack = importStack;
    data_input = &data;
    if (loop_data != nullptr) {
        additional_data = *loop_data;
        // TODO: do we actually want this?
        currentLoopData = std::make_shared<DataObject>(
            additional_data["loop"]
        );
    }

    template_stack.emplace_back(current_template);

    current_template->root.accept(*this);

    data_tmp_stack.clear();
}

bool Renderer::truthy(const DataObject *data) {
    if (data->is_bool()) {
        return data->get<bool>();
    } else if (data->is_number()) {
        return ((*data) != 0);
    } else if (data->is_null()) {
        return false;
    }
    return !data->empty();
}

void Renderer::print_data(const std::shared_ptr<DataObject> &value) {
    if (value->is_string()) {
        if (config.html_autoescape) {
            *output_stream << htmlescape(value->get<String>());
        } else {
            *output_stream << value->get<String>();
        }
    } else if (value->is_int()) {
        *output_stream << value->get<Int>();
    } else if (value->is_null()) {
    } else {
        *output_stream << value->serialize();
    }
}
const std::shared_ptr<DataObject>
Renderer::eval_expression_list(const ExpressionListNode &expression_list) {
    if (!expression_list.root) {
        throw_renderer_error("empty expression", expression_list);
    }

    expression_list.root->accept(*this);

    if (data_eval_stack.empty()) {
        throw_renderer_error("empty expression", expression_list);
    } else if (data_eval_stack.size() != 1) {
        throw_renderer_error("malformed expression", expression_list);
    }

    const auto result = data_eval_stack.top();
    data_eval_stack.pop();

    if (result == nullptr) {
        if (not_found_stack.empty()) {
            throw_renderer_error("expression could not be evaluated",
                expression_list);
        }

        const auto node = not_found_stack.top();
        not_found_stack.pop();

        throw_renderer_error("variable '" + static_cast<std::string>(node->name) +
            "' not found",
            *node);
    }
    return std::make_shared<DataObject>(*result);
}
void Renderer::throw_renderer_error(
    const std::string &message,
    const AstNode &node
) {
    SourceLocation loc = get_source_location(
        current_template->content,
        node.pos
    );
    loc.filePathOrIdentifier = this->current_template->pathOrIdentifier;
    loc.importStack = this->importStack;
    
    IRIS_THROW(RenderError(message, std::move(loc)));
}
void Renderer::make_result(
    const DataObject &&result
) {
    auto result_ptr = std::make_shared<DataObject>(result);
    data_tmp_stack.push_back(result_ptr);
    data_eval_stack.push(result_ptr.get());
}
void Renderer::visit(const BlockNode &node) {
    for (const auto &n : node.nodes) {
        n->accept(*this);

        if (break_rendering) {
            break;
        }
    }
}
void Renderer::visit(const TextNode &node) {
    output_stream->write(current_template->content.c_str() + node.pos,
        node.length);
}

void Renderer::visit(const ArrayNode &node) {
    auto object = std::make_shared<DataObject>(Array{});

    for (auto &child : node.children) {
        child->accept(*this);
        auto head = data_eval_stack.top();
        data_eval_stack.pop();

        object->push_back(*head);
    }
    data_tmp_stack.push_back(object);
    data_eval_stack.push(object.get());
}

void Renderer::visit(const ObjectNode& node) {
    auto object = std::make_shared<DataObject>(
        Object {}
    );

    for (auto& [k, v] : node.children) {
        v->accept(*this);

        auto head = data_eval_stack.top();
        data_eval_stack.pop();
        (*object)[k] = *head;
    }

    data_tmp_stack.push_back(object);
    data_eval_stack.push(object.get());
}
void Renderer::visit(const LiteralNode &node) {
    data_eval_stack.push(&node.value);
}

void Renderer::visit(const DataNode &node) {
    try {
        if (additional_data.contains(node.ptr)) {
            data_eval_stack.push(&(additional_data[node.ptr]));
        } else if (data_input->contains(node.ptr)) {
            data_eval_stack.push(&(*data_input).at(node.ptr));
        } else {
            // Try to evaluate as a no-argument callback
            const auto function_data = function_storage.find_function(node.name, 0);
            if (function_data.operation == FunctionStorage::Operation::Callback) {
                Arguments empty_args{};
                const auto value =
                    std::make_shared<DataObject>(function_data.callback(empty_args, udata));
                data_tmp_stack.push_back(value);
                data_eval_stack.push(value.get());
            } else {
                data_eval_stack.push(nullptr);
                not_found_stack.emplace(&node);
            }
        }
    } catch (const ContextlessRenderError& err) {
        throw_renderer_error(
            err.message,
            node
        );
    }
}
void Renderer::visit(const FunctionNode &node) {
    switch (node.operation) {
    case Op::Not: {
        const auto args = get_arguments<1>(node);
        make_result(!truthy(args[0]));
    } break;
    case Op::And: {
        make_result(truthy(get_arguments<1, 0>(node)[0]) &&
            truthy(get_arguments<1, 1>(node)[0]));
    } break;
    case Op::Or: {
        make_result(truthy(get_arguments<1, 0>(node)[0]) ||
            truthy(get_arguments<1, 1>(node)[0]));
    } break;
    case Op::In: {
        const auto args = get_arguments<2>(node);
        if (args[1]->is_array()) {
            auto &arr = args[1]->get<Array>();
            make_result(std::find(arr.begin(), arr.end(), *args[0]) != arr.end());
        } else {
            // TODO: support objects
            throw_renderer_error("The in operator is only defined for arrays. For "
                "objects, use exists or existsIn",
                node);
        }
    } break;
    case Op::Equal: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] == *args[1]);
    } break;
    case Op::NotEqual: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] != *args[1]);
    } break;
    case Op::Greater: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] > *args[1]);
    } break;
    case Op::GreaterEqual: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] >= *args[1]);
    } break;
    case Op::Less: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] < *args[1]);
    } break;
    case Op::LessEqual: {
        const auto args = get_arguments<2>(node);
        make_result(*args[0] <= *args[1]);
    } break;
    case Op::Add: {
        const auto args = get_arguments<2>(node);
        if (args[0]->is_string() && args[1]->is_string()) {
            make_result(args[0]->get<String>() + args[1]->get<String>());
        } else if (args[0]->is_int() && args[1]->is_int()) {
            make_result(args[0]->coerceInt() + args[1]->coerceInt());
        } else {
            make_result(args[0]->coerceFloat() + args[1]->coerceFloat());
        }
    } break;
    case Op::Subtract: {
        const auto args = get_arguments<2>(node);
        if (args[0]->is_int() && args[1]->is_int()) {
            make_result(args[0]->coerceInt() - args[1]->coerceInt());
        } else {
            make_result(args[0]->coerceFloat() - args[1]->coerceFloat());
        }
    } break;
    case Op::Multiplication: {
        const auto args = get_arguments<2>(node);
        if (args[0]->is_int() && args[1]->is_int()) {
            make_result(args[0]->coerceInt() * args[1]->coerceInt());
        } else {
            make_result(args[0]->coerceFloat() * args[1]->coerceFloat());
        }
    } break;
    case Op::Division: {
        const auto args = get_arguments<2>(node);
        if (args[1]->coerceFloat() == 0) {
            throw_renderer_error("division by zero", node);
        }
        make_result(args[0]->coerceFloat() / args[1]->coerceFloat());
    } break;
    case Op::Power: {
        const auto args = get_arguments<2>(node);
        if (args[0]->is_int() && args[1]->get<Int>() >= 0) {
            const auto result =
                (Int)std::pow(args[0]->coerceInt(), args[1]->get<Int>());
            make_result(result);
        } else {
            const auto result =
                std::pow(args[0]->coerceFloat(), args[1]->coerceFloat());
            make_result(result);
        }
    } break;
    case Op::Modulo: {
        const auto args = get_arguments<2>(node);
        make_result(args[0]->coerceInt() % args[1]->coerceInt());
    } break;
    case Op::AtId: {
        const auto container = get_arguments<1, 0, false>(node)[0];
        node.arguments[1]->accept(*this);
        if (not_found_stack.empty()) {
            throw_renderer_error("could not find element with given name", node);
        }
        const auto id_node = not_found_stack.top();
        not_found_stack.pop();
        data_eval_stack.pop();
        data_eval_stack.push(&container->at(id_node->name));
    } break;
    case Op::At: {
        const auto args = get_arguments<2>(node);
        if (args[0]->is_object()) {
            data_eval_stack.push(&args[0]->at(args[1]->get<String>()));
        } else {
            data_eval_stack.push(&args[0]->at(args[1]->get<Int>()));
        }
    } break;
    case Op::Capitalize: {
        auto result = get_arguments<1>(node)[0]->get<String>();
        if (result.size() == 0) {
            make_result("");
            return;
        }
        result[0] = static_cast<char>(::toupper(result[0]));
        std::transform(result.begin() + 1, result.end(), result.begin() + 1,
            [](char c) { return static_cast<char>(::tolower(c)); });
        make_result(std::move(result));
    } break;
    case Op::Default: {
        const auto test_arg = get_arguments<1, 0, false>(node)[0];
        data_eval_stack.push((test_arg != nullptr) ? test_arg
            : get_arguments<1, 1>(node)[0]);
    } break;
    case Op::DivisibleBy: {
        const auto args = get_arguments<2>(node);
        const auto divisor = args[1]->coerceInt();
        make_result((divisor != 0) && (args[0]->coerceInt() % divisor == 0));
    } break;
    case Op::Even: {
        make_result(get_arguments<1>(node)[0]->coerceInt() % 2 == 0);
    } break;
    case Op::Exists: {
        const auto& name = get_arguments<1>(node)[0]->get<String>();
        make_result(
            data_input->contains(DataPointer(name))
        );
    } break;
    case Op::ExistsInObject: {
        const auto args = get_arguments<2>(node);
        auto &&name = args[1]->get<String>();
        make_result(args[0]->contains(name));
    } break;
    case Op::First: {
        // ????
        const auto result = &get_arguments<1>(node)[0]->at(0);
        data_eval_stack.push(result);
    } break;
    case Op::Float: {
        make_result(std::stod(get_arguments<1>(node)[0]->get<String>()));
    } break;
    case Op::Int: {
        make_result(std::stoi(get_arguments<1>(node)[0]->get<String>()));
    } break;
    case Op::Last: {
        const auto result = &get_arguments<1>(node)[0]->back();
        data_eval_stack.push(result);
    } break;
    case Op::Length: {
        const auto val = get_arguments<1>(node)[0];
        if (val->is_string()) {
            make_result(val->get<String>().length());
        } else {
            make_result(val->size());
        }
    } break;
    case Op::Lower: {
        auto result = get_arguments<1>(node)[0]->get<String>();
        std::transform(result.begin(), result.end(), result.begin(),
            [](char c) { return static_cast<char>(::tolower(c)); });
        make_result(std::move(result));
    } break;
    case Op::Max: {
        const auto args = get_arguments<1>(node);
        const auto &it = args[0]->get<Array>();
        const auto result = std::max_element(it.begin(), it.end());
        data_eval_stack.push(&(*result));
    } break;
    case Op::Min: {
        const auto args = get_arguments<1>(node);
        const auto &it = args[0]->get<Array>();
        const auto result = std::min_element(it.begin(), it.end());
        data_eval_stack.push(&(*result));
    } break;
    case Op::Odd: {
        make_result(get_arguments<1>(node)[0]->coerceInt() % 2 != 0);
    } break;
    case Op::Range: {
        std::vector<DataObject> result(get_arguments<1>(node)[0]->coerceInt());
        std::iota(result.begin(), result.end(), 0);
        make_result(std::move(result));
    } break;
    case Op::Replace: {
        const auto args = get_arguments<3>(node);
        auto result = args[0]->get<std::string>();
        replace_substring(result, args[1]->get<std::string>(),
            args[2]->get<std::string>());
        make_result(std::move(result));
    } break;
    case Op::Round: {
        const auto args = get_arguments<2>(node);
        const auto precision = args[1]->coerceInt();
        const double result =
            std::round(args[0]->coerceFloat() * std::pow(10.0, precision)) /
            std::pow(10.0, precision);
        if (precision == 0) {
            make_result(static_cast<int>(result));
        } else {
            make_result(result);
        }
    } break;
    case Op::Sort: {
        auto result_ptr =
            std::make_shared<DataObject>(get_arguments<1>(node)[0]->get<Array>());
        auto &resultArray = result_ptr->get<Array>();
        std::sort(resultArray.begin(), resultArray.end());
        data_tmp_stack.push_back(result_ptr);
        data_eval_stack.push(result_ptr.get());
    } break;
    case Op::Upper: {
        auto result = get_arguments<1>(node)[0]->get<String>();
        std::transform(result.begin(), result.end(), result.begin(),
            [](char c) { return static_cast<char>(::toupper(c)); });
        make_result(std::move(result));
    } break;
    case Op::IsBoolean: {
        make_result(get_arguments<1>(node)[0]->is_bool());
    } break;
    case Op::IsNumber: {
        make_result(get_arguments<1>(node)[0]->is_number());
    } break;
    case Op::IsInteger: {
        make_result(get_arguments<1>(node)[0]->is_int());
    } break;
    case Op::IsFloat: {
        make_result(get_arguments<1>(node)[0]->is_float());
    } break;
    case Op::IsObject: {
        make_result(get_arguments<1>(node)[0]->is_object());
    } break;
    case Op::IsArray: {
        make_result(get_arguments<1>(node)[0]->is_array());
    } break;
    case Op::IsString: {
        make_result(get_arguments<1>(node)[0]->is_string());
    } break;
    case Op::Callback: {
        try {
        auto args = get_argument_vector(node);
        make_result(node.callback(args, udata));
        } catch (const ContextlessRenderError& err) {
            throw_renderer_error(
                err.message,
                node
            );
        }
    } break;
    case Op::Super: {
        const auto args = get_argument_vector(node);
        const size_t old_level = current_level;
        const size_t level_diff = (args.size() == 1) ? args[0]->get<Int>() : 1;
        const size_t level = current_level + level_diff;

        if (block_statement_stack.empty()) {
            throw_renderer_error("super() call is not within a block", node);
        }

        if (level < 1 || level > template_stack.size() - 1) {
            throw_renderer_error("level of super() call does not match parent "
                "templates (between 1 and " +
                std::to_string(template_stack.size() - 1) + ")",
                node);
        }

        const auto current_block_statement = block_statement_stack.back();
        const Template *new_template = template_stack.at(level);
        const Template *old_template = current_template;
        const auto block_it =
            new_template->block_storage.find(current_block_statement->name);
        if (block_it != new_template->block_storage.end()) {
            current_template = new_template;
            current_level = level;
            block_it->second->block.accept(*this);
            current_level = old_level;
            current_template = old_template;
        } else {
            throw_renderer_error("could not find block with name '" +
                current_block_statement->name + "'",
                node);
        }
        make_result(nullptr);
    } break;
    case Op::Join: {
        const auto args = get_arguments<2>(node);
        const auto separator = args[1]->get<String>();
        std::ostringstream os;
        std::string sep;
        for (const auto &value : args[0]->get<Array>()) {
            os << sep;
            os << value.serialize();
            sep = separator;
        }
        make_result(os.str());
    } break;
    case Op::None:
        break;
    }
}
void Renderer::visit(const ExpressionListNode &node) {
    print_data(eval_expression_list(node));
}
void Renderer::visit(const StatementNode &) {}
void Renderer::visit(const ForStatementNode &) {}
void Renderer::visit(const ForArrayStatementNode &node) {
    const auto result = eval_expression_list(node.condition);
    if (!result->is_array()) {
        throw_renderer_error("object must be an array", node);
    }

    if (currentLoopData != nullptr) {
        DataObject old(*currentLoopData);
        currentLoopData = std::make_shared<DataObject>(iris::Object{});
        (*currentLoopData)["parent"] = std::move(old);
    } else {
        currentLoopData = std::make_shared<DataObject>(iris::Object{});
    }

    size_t index = 0;
    (*currentLoopData)["is_first"] = true;
    (*currentLoopData)["is_last"] = (result->size() <= 1);
    auto &array = result->get<Array>();
    for (auto &it : array) {
        additional_data[static_cast<std::string>(node.value)] = it;

        (*currentLoopData)["index"] = index;
        (*currentLoopData)["index1"] = index + 1;
        if (index == 1) {
            (*currentLoopData)["is_first"] = false;
        }
        if (index == result->size() - 1) {
            (*currentLoopData)["is_last"] = true;
        }

        // TODO: this is less efficient but is a forced hack, since the pointers to objects currently aren't stable with
        // iteration.
        additional_data["loop"] = *currentLoopData;

        node.body.accept(*this);
        ++index;
    }

    additional_data[node.value].clear();
    if (currentLoopData->contains("parent")) {
        const auto tmp = (*currentLoopData)["parent"];
        *currentLoopData = tmp;
    } else {
        currentLoopData = nullptr;
        additional_data["loop"] = nullptr;
    }
}
void Renderer::visit(const ForObjectStatementNode &node) {
    const auto result = eval_expression_list(node.condition);
    if (!result->is_object()) {
        throw_renderer_error("object must be an object", node);
    }

    if (currentLoopData != nullptr) {
        DataObject old(*currentLoopData);
        currentLoopData = std::make_shared<DataObject>(iris::Object{});
        (*currentLoopData)["parent"] = std::move(old);
    } else {
        currentLoopData = std::make_shared<DataObject>(iris::Object{});
    }

    size_t index = 0;
    (*currentLoopData)["is_first"] = true;
    (*currentLoopData)["is_last"] = (result->size() <= 1);
    auto &obj = result->get<Object>();
    for (const auto &[k, v] : obj) {
        additional_data[static_cast<std::string>(node.key)] = k;
        additional_data[static_cast<std::string>(node.value)] = v;

        (*currentLoopData)["index"] = index;
        (*currentLoopData)["index1"] = index + 1;
        if (index == 1) {
            (*currentLoopData)["is_first"] = false;
        }
        if (index == result->size() - 1) {
            (*currentLoopData)["is_last"] = true;
        }

        // TODO: this is less efficient but is a forced hack, since the pointers to objects currently aren't stable with
        // iteration.
        additional_data["loop"] = *currentLoopData;

        node.body.accept(*this);
        ++index;
    }

    additional_data[static_cast<std::string>(node.key)].clear();
    additional_data[static_cast<std::string>(node.value)].clear();
    if (currentLoopData->contains("parent")) {
        const auto tmp = (*currentLoopData)["parent"];
        *currentLoopData = tmp;
    } else {
        currentLoopData = nullptr;
        additional_data["loop"] = nullptr;
    }
}
void Renderer::visit(const IfStatementNode &node) {
    const auto result = eval_expression_list(node.condition);
    if (truthy(result.get())) {
        node.true_statement.accept(*this);
    } else if (node.has_false_statement) {
        node.false_statement.accept(*this);
    }
}

std::stack<SourceLocation> Renderer::appendSourceLocStack(
    const AstNode* node
) {
    auto stack = importStack;

    auto loc = get_source_location(
        current_template->content,
        node->pos
    );
    loc.filePathOrIdentifier = current_template->pathOrIdentifier;
    
    stack.push(loc);
    // I don't think this will ever trip, but `auto` always feels a bit murky. Will only be checked in debug mode
    // anyway, so I don't really care
    assert(stack.size() != importStack.size());

    return stack;
}

}
