#include "parser.hpp"
#include <format>
#include <memory>

namespace iris {

void Parser::throw_parser_error(const std::string &message) const {
    IRIS_THROW(ParserError(message, lexer.current_position()));
}
void Parser::get_next_token() {
    if (have_peek_tok) {
        tok = peek_tok;
        have_peek_tok = false;
    } else {
        tok = lexer.scan();
    }
}
void Parser::get_peek_token() {
    if (!have_peek_tok) {
        peek_tok = lexer.scan();
        have_peek_tok = true;
    }
}
std::string_view Parser::getDataText() {
    return std::string_view(
        literal_start.data(), // NOLINT(bugprone-suspicious-stringview-data-usage): properly delimited by length
        tok.text.data() - literal_start.data() + tok.text.size()
    );
}
void Parser::add_operator(Arguments &arguments, OperatorStack &operator_stack) {
    auto function = operator_stack.top();
    operator_stack.pop();

    if (static_cast<int>(arguments.size()) < function->number_args) {
        throw_parser_error("too few arguments");
    }

    for (int i = 0; i < function->number_args; ++i) {
        function->arguments.insert(function->arguments.begin(), arguments.back());
        arguments.pop_back();
    }
    arguments.emplace_back(function);
}
void Parser::add_to_template_storage(const std::filesystem::path &path,
    std::string &template_name) {
    if (template_storage.find(template_name) != template_storage.end()) {
        return;
    }

    const std::string original_name = template_name;

    if (config.search_included_templates_in_files) {
        // Build the relative path
        template_name = (path / original_name).string();
        if (template_name.compare(0, 2, "./") == 0) {
            template_name.erase(0, 2);
        }

        if (template_storage.find(template_name) == template_storage.end()) {
            // Load file
            std::ifstream file;
            file.open(template_name);
            if (!file.fail()) {
                const std::string text((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());

                auto include_template = Template(text);
                template_storage.emplace(template_name, include_template);
                parse_into_template(template_storage[template_name], template_name);
                return;
            } else if (!config.include_callback) {
                IRIS_THROW(
                    FileError("failed accessing file at '" + template_name + "'"));
            }
        }
    }

    // Try include callback
    if (config.include_callback) {
        auto include_template = config.include_callback(path, original_name);
        template_storage.emplace(template_name, include_template);
    }
}
std::string Parser::parse_filename() const {
    if (tok.kind != Token::Kind::String) {
        throw_parser_error("expected string, got '" + tok.describe() + "'");
    }

    if (tok.text.length() < 2) {
        throw_parser_error("expected filename, got '" +
            static_cast<std::string>(tok.text) + "'");
    }

    // Remove first and last character ""
    return std::string{tok.text.substr(1, tok.text.length() - 2)};
}
bool Parser::parse_expression(Template &tmpl, Token::Kind closing) {
    current_expression_list->root = parse_expression(tmpl);
    return tok.kind == closing;
}

void Parser::pushExpressionNode(
    Arguments &args,
    std::shared_ptr<ExpressionNode> &&node,
    std::stack<LiteralBlockType>& objects
) {
    if (objects.empty()) {
        args.emplace_back(node);
    } else {
        auto &parentNode = args.back();
        if (auto arrNode = std::dynamic_pointer_cast<ArrayNode>(parentNode)) {
            arrNode->children.push_back(std::move(node));
        } else {
            auto litNode = std::dynamic_pointer_cast<LiteralNode>(parentNode);
            if (!litNode) {
                throw_parser_error(
                    "invalid key-value pair in object"
                );
            }

            if (!litNode->value.is_string()) {
                throw_parser_error(
                    std::format(
                        "Expected object key of type string, but found {}",
                        litNode->value.type()
                    )
                );
            }

            args.pop_back();

            auto objNode = std::dynamic_pointer_cast<ObjectNode>(args.back());

            if (objNode == nullptr) {
                throw_parser_error("bug: expression expected ObjectNode but found unrelated type");
            }
            objNode->children[litNode->value.get<String>()] = std::move(node);
        }
    }
}
std::shared_ptr<ExpressionNode> Parser::parse_expression(Template &tmpl) {
    Arguments arguments;
    std::stack<OperatorStack> operator_stacks;
    operator_stacks.push({}); // Top-level operator stack

    std::stack<LiteralBlockType> objects;

    // std::stack<std::string> objectKeys;
    // std::stack<std::variant<std::shared_ptr<ObjectNode>, std::shared_ptr<ArrayNode>>> objectStack;
    // size_t objectsInObjectStack = 0;

    while (tok.kind != Token::Kind::Eof) {
        // Literals
        switch (tok.kind) {
        case Token::Kind::String: {
            literal_start = tok.text;

            auto body = getDataText();

            if (!body.starts_with('"') || !body.ends_with('"')) {
                throw_parser_error("Illegal string");
            }

            DataObject root = std::string(
                body.size() > 2 ? body.substr(1, body.size() - 2) : ""
            );
            auto node = std::make_shared<LiteralNode>(
                std::move(root),
                body.data() - tmpl.content.c_str()
            );

            arguments.push_back(node);
            // pushExpressionNode(arguments, node, objectKeys, objectStack, objectsInObjectStack, true);
        } break;
        case Token::Kind::Number: {
            literal_start = tok.text;

            auto body = std::string(getDataText());
            DataObject obj;
            size_t ptr;
            try {
                if (body.find('.') != std::string_view::npos) {
                    auto value = std::stod(body, &ptr);
                    if (ptr != body.size()) {
                        throw_parser_error(
                            std::format(
                                "Illegal double: {}",
                                body
                            )
                        );
                    }
                    obj = value;
                } else {
                    auto value = std::stoll(body, &ptr);
                    if (ptr != body.size()) {
                        throw_parser_error(
                            std::format(
                                "Illegal integer: {}",
                                body
                            )
                        );
                    }
                    obj = value;
                }
            } catch (const std::invalid_argument& e) {
                throw_parser_error(
                    std::format(
                        "{} could not be parsed as a legal number. Native error: {}",
                        body,
                        e.what()
                    )
                );
            } catch (const std::out_of_range& e) {
                throw_parser_error(
                    std::format(
                        "The number {} is too big to be represented by this type. Native error: {}",
                        body,
                        e.what()
                    )
                );
            }
            arguments.emplace_back(
                std::make_shared<LiteralNode>(
                    std::move(obj),
                    body.data() - tmpl.content.c_str()
                )
            );
        } break;
        case Token::Kind::LeftBracket: { // [
            if (objects.empty()) {
                // TODO: I think this is wrong?
                literal_start = tok.text;
            }
            arguments.emplace_back(
                std::make_shared<ArrayNode>(literal_start.data() - tmpl.content.c_str())
            );
            objects.push(LiteralBlockType::Array);
            operator_stacks.push({});
        } break;
        case Token::Kind::LeftBrace: { // {
            if (objects.empty()) {
                literal_start = tok.text;
            }
            arguments.emplace_back(std::make_shared<ObjectNode>(literal_start.data() -
                    tmpl.content.c_str()));
            objects.push(LiteralBlockType::Object);
            operator_stacks.push({});
        } break;
        case Token::Kind::RightBracket: {
            if (objects.empty()) {
                // TODO: what consequences does disabling this throw have? It's necessary for IDs to be referencable in
                // dicts and arrays
                // throw_parser_error("unexpected ']'");
                goto break_loop;
            } else if (objects.top() != LiteralBlockType::Array) {
                throw_parser_error("unexpected ']'");
            }

            processOperators(arguments, operator_stacks);
            operator_stacks.pop();

            auto possibleArg = arguments.back();
            auto arrNode = std::dynamic_pointer_cast<ArrayNode>(possibleArg);
            arguments.pop_back();

            if (arrNode == nullptr) {
                arrNode = std::dynamic_pointer_cast<ArrayNode>(arguments.back());
                if (arrNode == nullptr) {
                    throw_parser_error(
                        "Syntax error: expected ']', found invalid expression"
                    );
                }
                arguments.pop_back();

                arrNode->children.push_back(possibleArg);
            }

            objects.pop();
            pushExpressionNode(arguments, arrNode, objects);
        } break;
        case Token::Kind::RightBrace: {
            if (objects.empty()) {
                // TODO: what consequences does disabling this throw have? It's necessary for IDs to be referencable in
                // dicts and arrays
                // throw_parser_error("unexpected '}'");
                goto break_loop;
            } else if (objects.top() != LiteralBlockType::Object) {
                throw_parser_error("unexpected '}'");
            }

            processOperators(arguments, operator_stacks);
            operator_stacks.pop();

            auto possibleArg = arguments.back();
            auto objNode = std::dynamic_pointer_cast<ObjectNode>(possibleArg);
            arguments.pop_back();

            if (objNode == nullptr) {
                auto possibleKey = std::dynamic_pointer_cast<LiteralNode>(arguments.back());
                if (possibleKey == nullptr) {
                    throw_parser_error(
                        "final entry in array is not a key-value pair"
                    );
                }
                arguments.pop_back();
                
                objNode = std::dynamic_pointer_cast<ObjectNode>(arguments.back());
                if (objNode == nullptr) {
                    throw_parser_error(
                        "Syntax error: expected '}', found invalid expression"
                    );
                }
                arguments.pop_back();

                objNode->children[possibleKey->value.get<String>()] = possibleArg;
            }

            objects.pop();
            pushExpressionNode(arguments, objNode, objects);
        } break;
        case Token::Kind::Id: {
            // if (!objectStack.empty()) {
            //     pushExpressionNode(
            //         arguments,
            //         parse_expression(tmpl),
            //         objectKeys,
            //         objectStack,
            //         objectsInObjectStack
            //     );
            //     continue;
            // }
            get_peek_token();

            // Data Literal
            if (tok.text == static_cast<decltype(tok.text)>("true") ||
                tok.text == static_cast<decltype(tok.text)>("false") ||
                tok.text == static_cast<decltype(tok.text)>("null")) {
                literal_start = tok.text;
                auto body = getDataText();
                DataObject root;

                if (tok.text == "false" || tok.text == "true") {
                    root = tok.text == "true";
                } else {
                    // Technically redundant, but this feels clearer
                    root = nullptr;
                }
                arguments.emplace_back(std::make_shared<LiteralNode>(
                        std::move(root), body.data() - tmpl.content.c_str()));

                // Operator
            } else if (tok.text == "and" || tok.text == "or" || tok.text == "in" ||
                tok.text == "not") {
                goto parse_operator;

                // Functions
            } else if (peek_tok.kind == Token::Kind::LeftParen) {
                auto func = std::make_shared<FunctionNode>(
                    tok.text, tok.text.data() - tmpl.content.c_str());
                get_next_token();
                do {
                    get_next_token();
                    auto expr = parse_expression(tmpl);
                    if (!expr) {
                        break;
                    }
                    func->number_args += 1;
                    func->arguments.emplace_back(expr);
                } while (tok.kind == Token::Kind::Comma);
                if (tok.kind != Token::Kind::RightParen) {
                    throw_parser_error("expected right parenthesis, got '" +
                        tok.describe() + "'");
                }

                auto function_data =
                    function_storage.find_function(func->name, func->number_args);
                if (function_data.operation == FunctionStorage::Operation::None) {
                    throw_parser_error("unknown function " + func->name);
                }
                func->operation = function_data.operation;
                if (function_data.operation == FunctionStorage::Operation::Callback) {
                    func->callback = function_data.callback;
                }
                arguments.emplace_back(func);

                // Variables
            } else {
                arguments.emplace_back(
                    std::make_shared<DataNode>(static_cast<std::string>(tok.text),
                        tok.text.data() - tmpl.content.c_str()));
            }

            // Operators
        } break;
        case Token::Kind::Equal:
        case Token::Kind::NotEqual:
        case Token::Kind::GreaterThan:
        case Token::Kind::GreaterEqual:
        case Token::Kind::LessThan:
        case Token::Kind::LessEqual:
        case Token::Kind::Plus:
        case Token::Kind::Minus:
        case Token::Kind::Times:
        case Token::Kind::Slash:
        case Token::Kind::Power:
        case Token::Kind::Percent:
        case Token::Kind::Dot: {

            parse_operator:
            FunctionStorage::Operation operation;
            switch (tok.kind) {
            case Token::Kind::Id: {
                if (tok.text == "and") {
                    operation = FunctionStorage::Operation::And;
                } else if (tok.text == "or") {
                    operation = FunctionStorage::Operation::Or;
                } else if (tok.text == "in") {
                    operation = FunctionStorage::Operation::In;
                } else if (tok.text == "not") {
                    operation = FunctionStorage::Operation::Not;
                } else {
                    throw_parser_error("unknown operator in parser.");
                }
            } break;
            case Token::Kind::Equal: {
                operation = FunctionStorage::Operation::Equal;
            } break;
            case Token::Kind::NotEqual: {
                operation = FunctionStorage::Operation::NotEqual;
            } break;
            case Token::Kind::GreaterThan: {
                operation = FunctionStorage::Operation::Greater;
            } break;
            case Token::Kind::GreaterEqual: {
                operation = FunctionStorage::Operation::GreaterEqual;
            } break;
            case Token::Kind::LessThan: {
                operation = FunctionStorage::Operation::Less;
            } break;
            case Token::Kind::LessEqual: {
                operation = FunctionStorage::Operation::LessEqual;
            } break;
            case Token::Kind::Plus: {
                operation = FunctionStorage::Operation::Add;
            } break;
            case Token::Kind::Minus: {
                operation = FunctionStorage::Operation::Subtract;
            } break;
            case Token::Kind::Times: {
                operation = FunctionStorage::Operation::Multiplication;
            } break;
            case Token::Kind::Slash: {
                operation = FunctionStorage::Operation::Division;
            } break;
            case Token::Kind::Power: {
                operation = FunctionStorage::Operation::Power;
            } break;
            case Token::Kind::Percent: {
                operation = FunctionStorage::Operation::Modulo;
            } break;
            case Token::Kind::Dot: {
                operation = FunctionStorage::Operation::AtId;
            } break;
            default: {
                throw_parser_error("unknown operator in parser.");
            }
            }
            auto function_node = std::make_shared<FunctionNode>(
                operation, tok.text.data() - tmpl.content.c_str());


            auto& operator_stack = operator_stacks.top();
            while (!operator_stack.empty() &&
                ((operator_stack.top()->precedence > function_node->precedence) ||
                    (operator_stack.top()->precedence == function_node->precedence &&
                        function_node->associativity ==
                        FunctionNode::Associativity::Left))) {
                add_operator(arguments, operator_stack);
            }

            operator_stack.emplace(function_node);
        } break;
        case Token::Kind::Comma: {
            if (objects.empty()) {
                goto break_loop;
            } else {
                auto arg = arguments.back();
                arguments.pop_back();
                pushExpressionNode(arguments, std::move(arg), objects);
            }
        } break;
        case Token::Kind::Colon: {
            if (objects.empty()) {
                throw_parser_error("unexpected ':'");
            }
            processOperators(arguments, operator_stacks);
            const auto& back = arguments.back();
            if (auto n = std::dynamic_pointer_cast<LiteralNode>(back); n != nullptr) {
                if (!n->value.is_string()) {
                    throw_parser_error("Object got non-string key");
                }
                const auto& obj = arguments.at(arguments.size() - 2);

                if (std::dynamic_pointer_cast<ObjectNode>(obj) == nullptr) {
                    throw_parser_error("Found ':' in non-object");
                }
            } else {
                throw_parser_error("unexpected ':': no key defined");
            }
        } break;
        case Token::Kind::LeftParen: {
            get_next_token();
            auto expr = parse_expression(tmpl);
            if (tok.kind != Token::Kind::RightParen) {
                throw_parser_error("expected right parenthesis, got '" +
                    tok.describe() + "'");
            }
            if (!expr) {
                throw_parser_error("empty expression in parentheses");
            }
            arguments.emplace_back(expr);
        } break;

            // parse function call pipe syntax
        case Token::Kind::Pipe: {
            // get function name
            get_next_token();
            if (tok.kind != Token::Kind::Id) {
                throw_parser_error("expected function name, got '" + tok.describe() +
                    "'");
            }
            auto func = std::make_shared<FunctionNode>(
                tok.text, tok.text.data() - tmpl.content.c_str());
            // add first parameter as last value from arguments
            func->number_args += 1;
            func->arguments.emplace_back(arguments.back());
            arguments.pop_back();
            get_peek_token();
            if (peek_tok.kind == Token::Kind::LeftParen) {
                get_next_token();
                // parse additional parameters
                do {
                    get_next_token();
                    auto expr = parse_expression(tmpl);
                    if (!expr) {
                        break;
                    }
                    func->number_args += 1;
                    func->arguments.emplace_back(expr);
                } while (tok.kind == Token::Kind::Comma);
                if (tok.kind != Token::Kind::RightParen) {
                    throw_parser_error("expected right parenthesis, got '" +
                        tok.describe() + "'");
                }
            }
            // search store for defined function with such name and number of args
            auto function_data =
                function_storage.find_function(func->name, func->number_args);
            if (function_data.operation == FunctionStorage::Operation::None) {
                throw_parser_error("unknown function " + func->name);
            }
            func->operation = function_data.operation;
            if (function_data.operation == FunctionStorage::Operation::Callback) {
                func->callback = function_data.callback;
            }
            arguments.emplace_back(func);
        } break;
        default:
            goto break_loop;
        }

        get_next_token();
    }

 break_loop:
    if (!objects.empty()) {
        throw_parser_error("malformed expression (unterminated object or array)");
    }

    if (operator_stacks.size() != 1) {
        throw_parser_error(
            "Malformed expression: uncaught unterminated sub-block"
        );
    }

    processOperators(arguments, operator_stacks);
    operator_stacks.pop();

    std::shared_ptr<ExpressionNode> expr;
    if (arguments.size() == 1) {
        expr = arguments[0];
        arguments = {};
    } else if (arguments.size() > 1) {
        throw_parser_error("malformed expression");
    }
    return expr;
}

void Parser::processOperators(Arguments& arguments, std::stack<OperatorStack>& operatorStacks) {
    auto& stack = operatorStacks.top();
    while (!stack.empty()) {
        add_operator(arguments, stack);
    }
}

bool Parser::parse_statement(Template &tmpl, Token::Kind closing,
    const std::filesystem::path &path) {
    if (tok.kind != Token::Kind::Id) {
        return false;
    }

    if (tok.text == static_cast<decltype(tok.text)>("if")) {
        get_next_token();

        auto if_statement_node = std::make_shared<IfStatementNode>(
            current_block, tok.text.data() - tmpl.content.c_str());
        current_block->nodes.emplace_back(if_statement_node);
        if_statement_stack.emplace(if_statement_node.get());
        current_block = &if_statement_node->true_statement;
        current_expression_list = &if_statement_node->condition;

        if (!parse_expression(tmpl, closing)) {
            return false;
        }
    } else if (tok.text == static_cast<decltype(tok.text)>("else")) {
        if (if_statement_stack.empty()) {
            throw_parser_error("else without matching if");
        }
        auto &if_statement_data = if_statement_stack.top();
        get_next_token();

        if_statement_data->has_false_statement = true;
        current_block = &if_statement_data->false_statement;

        // Chained else if
        if (tok.kind == Token::Kind::Id &&
            tok.text == static_cast<decltype(tok.text)>("if")) {
            get_next_token();

            auto if_statement_node = std::make_shared<IfStatementNode>(
                true, current_block, tok.text.data() - tmpl.content.c_str());
            current_block->nodes.emplace_back(if_statement_node);
            if_statement_stack.emplace(if_statement_node.get());
            current_block = &if_statement_node->true_statement;
            current_expression_list = &if_statement_node->condition;

            if (!parse_expression(tmpl, closing)) {
                return false;
            }
        }
    } else if (tok.text == static_cast<decltype(tok.text)>("endif")) {
        if (if_statement_stack.empty()) {
            throw_parser_error("endif without matching if");
        }

        // Nested if statements
        while (if_statement_stack.top()->is_nested) {
            if_statement_stack.pop();
        }

        auto &if_statement_data = if_statement_stack.top();
        get_next_token();

        current_block = if_statement_data->parent;
        if_statement_stack.pop();
    } else if (tok.text == static_cast<decltype(tok.text)>("block")) {
        get_next_token();

        if (tok.kind != Token::Kind::Id) {
            throw_parser_error("expected block name, got '" + tok.describe() + "'");
        }

        const std::string block_name = static_cast<std::string>(tok.text);

        auto block_statement_node = std::make_shared<BlockStatementNode>(
            current_block, block_name, tok.text.data() - tmpl.content.c_str());
        current_block->nodes.emplace_back(block_statement_node);
        block_statement_stack.emplace(block_statement_node.get());
        current_block = &block_statement_node->block;
        auto success = tmpl.block_storage.emplace(block_name, block_statement_node);
        if (!success.second) {
            throw_parser_error("block with the name '" + block_name +
                "' does already exist");
        }

        get_next_token();
    } else if (tok.text == static_cast<decltype(tok.text)>("endblock")) {
        if (block_statement_stack.empty()) {
            throw_parser_error("endblock without matching block");
        }

        auto &block_statement_data = block_statement_stack.top();
        get_next_token();

        current_block = block_statement_data->parent;
        block_statement_stack.pop();
    } else if (tok.text == static_cast<decltype(tok.text)>("for")) {
        get_next_token();

        // options: for a in arr; for a, b in obj
        if (tok.kind != Token::Kind::Id) {
            throw_parser_error("expected id, got '" + tok.describe() + "'");
        }

        Token value_token = tok;
        get_next_token();

        // Object type
        std::shared_ptr<ForStatementNode> for_statement_node;
        if (tok.kind == Token::Kind::Comma) {
            get_next_token();
            if (tok.kind != Token::Kind::Id) {
                throw_parser_error("expected id, got '" + tok.describe() + "'");
            }

            const Token key_token = value_token;
            value_token = tok;
            get_next_token();

            for_statement_node = std::make_shared<ForObjectStatementNode>(
                static_cast<std::string>(key_token.text),
                static_cast<std::string>(value_token.text), current_block,
                tok.text.data() - tmpl.content.c_str());

            // Array type
        } else {
            for_statement_node = std::make_shared<ForArrayStatementNode>(
                static_cast<std::string>(value_token.text), current_block,
                tok.text.data() - tmpl.content.c_str());
        }

        current_block->nodes.emplace_back(for_statement_node);
        for_statement_stack.emplace(for_statement_node.get());
        current_block = &for_statement_node->body;
        current_expression_list = &for_statement_node->condition;

        if (tok.kind != Token::Kind::Id ||
            tok.text != static_cast<decltype(tok.text)>("in")) {
            throw_parser_error("expected 'in', got '" + tok.describe() + "'");
        }
        get_next_token();

        if (!parse_expression(tmpl, closing)) {
            return false;
        }
    } else if (tok.text == static_cast<decltype(tok.text)>("endfor")) {
        if (for_statement_stack.empty()) {
            throw_parser_error("endfor without matching for");
        }

        auto &for_statement_data = for_statement_stack.top();
        get_next_token();

        current_block = for_statement_data->parent;
        for_statement_stack.pop();
    } else if (tok.text == static_cast<decltype(tok.text)>("include")) {
        get_next_token();

        std::string template_name = parse_filename();
        add_to_template_storage(path, template_name);

        current_block->nodes.emplace_back(std::make_shared<IncludeStatementNode>(
                template_name, tok.text.data() - tmpl.content.c_str()));

        get_next_token();
    } else if (tok.text == static_cast<decltype(tok.text)>("extends")) {
        get_next_token();

        std::string template_name = parse_filename();
        add_to_template_storage(path, template_name);

        current_block->nodes.emplace_back(std::make_shared<ExtendsStatementNode>(
                template_name, tok.text.data() - tmpl.content.c_str()));

        get_next_token();
    } else if (tok.text == static_cast<decltype(tok.text)>("set")) {
        get_next_token();

        if (tok.kind != Token::Kind::Id) {
            throw_parser_error("expected variable name, got '" + tok.describe() +
                "'");
        }

        const std::string key = static_cast<std::string>(tok.text);
        get_next_token();

        auto set_statement_node = std::make_shared<SetStatementNode>(
            key, tok.text.data() - tmpl.content.c_str());
        current_block->nodes.emplace_back(set_statement_node);
        current_expression_list = &set_statement_node->expression;

        if (tok.text != static_cast<decltype(tok.text)>("=")) {
            throw_parser_error("expected '=', got '" + tok.describe() + "'");
        }
        get_next_token();

        if (!parse_expression(tmpl, closing)) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}
void Parser::parse_into(Template &tmpl, const std::filesystem::path &path) {
    lexer.start(tmpl.content);
    current_block = &tmpl.root;

    for (;;) {
        get_next_token();
        switch (tok.kind) {
        case Token::Kind::Eof: {
            if (!if_statement_stack.empty()) {
                throw_parser_error("unmatched if");
            }
            if (!for_statement_stack.empty()) {
                throw_parser_error("unmatched for");
            }
        }
            current_block = nullptr;
            return;
        case Token::Kind::Text: {
            current_block->nodes.emplace_back(std::make_shared<TextNode>(
                    tok.text.data() - tmpl.content.c_str(), tok.text.size()));
        } break;
        case Token::Kind::StatementOpen: {
            get_next_token();
            if (!parse_statement(tmpl, Token::Kind::StatementClose, path)) {
                throw_parser_error("expected statement, got '" + tok.describe() + "'");
            }
            if (tok.kind != Token::Kind::StatementClose) {
                throw_parser_error("expected statement close, got '" + tok.describe() +
                    "'");
            }
        } break;
        case Token::Kind::LineStatementOpen: {
            get_next_token();
            if (!parse_statement(tmpl, Token::Kind::LineStatementClose, path)) {
                throw_parser_error("expected statement, got '" + tok.describe() + "'");
            }
            if (tok.kind != Token::Kind::LineStatementClose &&
                tok.kind != Token::Kind::Eof) {
                throw_parser_error("expected line statement close, got '" +
                    tok.describe() + "'");
            }
        } break;
        case Token::Kind::ExpressionOpen: {
            get_next_token();

            auto expression_list_node = std::make_shared<ExpressionListNode>(
                tok.text.data() - tmpl.content.c_str());
            current_block->nodes.emplace_back(expression_list_node);
            current_expression_list = expression_list_node.get();

            if (!parse_expression(tmpl, Token::Kind::ExpressionClose)) {
                throw_parser_error("expected expression close, got '" + tok.describe() +
                    "'");
            }
        } break;
        case Token::Kind::CommentOpen: {
            get_next_token();
            if (tok.kind != Token::Kind::CommentClose) {
                throw_parser_error("expected comment close, got '" + tok.describe() +
                    "'");
            }
        } break;
        default: {
            throw_parser_error("unexpected token '" + tok.describe() + "'");
        } break;
        }
    }
}
Parser::Parser(const ParserConfig &parser_config,
    const LexerConfig &lexer_config,
    TemplateStorage &template_storage,
    const FunctionStorage &function_storage)
    : config(parser_config), lexer(lexer_config),
      template_storage(template_storage), function_storage(function_storage) {}
Template Parser::parse(std::string_view input,
    const std::filesystem::path &path) {
    auto result = Template(std::string(input));
    parse_into(result, path);
    return result;
}
void Parser::parse_into_template(Template &tmpl,
    const std::filesystem::path &filename) {
    auto sub_parser = Parser(
        config,
        lexer.get_config(),
        template_storage,
        function_storage
    );
    sub_parser.parse_into(tmpl, filename.parent_path());
}

std::string Parser::load_file(const std::filesystem::path &filename) {
    std::ifstream file;
    file.open(filename);
    if (file.fail()) {
        IRIS_THROW(
            FileError("failed accessing file at '" + filename.string() + "'")
        );
    }
    std::string text((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    return text;
}
}
