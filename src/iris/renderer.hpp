#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

#include "config.hpp"
#include "function_storage.hpp"
#include "iris/data/DataObject.hpp"
#include "iris/data/DataPointer.hpp"
#include "node.hpp"
#include "template.hpp"

namespace iris {

/*!
  @brief Escapes HTML
*/
inline std::string htmlescape(const std::string& data) {
    std::string buffer;
    buffer.reserve(static_cast<size_t>(1.1 * data.size()));
    for (size_t pos = 0; pos != data.size(); ++pos) {
        switch (data[pos]) {
        case '&':  buffer.append("&amp;");       break;
        case '\"': buffer.append("&quot;");      break;
        case '\'': buffer.append("&apos;");      break;
        case '<':  buffer.append("&lt;");        break;
        case '>':  buffer.append("&gt;");        break;
        default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}

/*!
 * \brief Class for rendering a Template with data.
 */
class Renderer : public NodeVisitor {
private:
    using Op = FunctionStorage::Operation;

    const RenderConfig config;
    const TemplateStorage& template_storage;
    const FunctionStorage& function_storage;

    const Template* current_template = nullptr;
    size_t current_level {0};
    std::vector<const Template*> template_stack;
    std::vector<const BlockStatementNode*> block_statement_stack;

    const DataObject* data_input = nullptr;
    std::ostream* output_stream = nullptr;

    DataObject additional_data = Object {};
    std::shared_ptr<DataObject> currentLoopData = nullptr;

    std::vector<std::shared_ptr<DataObject>> data_tmp_stack;
    std::stack<const DataObject*> data_eval_stack;
    std::stack<const DataNode*> not_found_stack;

    bool break_rendering {false};

    static bool truthy(const DataObject *data);

    void print_data(const std::shared_ptr<DataObject> &value);

    const std::shared_ptr<DataObject>
    eval_expression_list(const ExpressionListNode &expression_list);

    void throw_renderer_error(const std::string &message, const AstNode &node);

    void make_result(const DataObject &&result);

    template <size_t N, size_t N_start = 0, bool throw_not_found = true>
    std::array<const DataObject*, N> get_arguments(const FunctionNode& node) {
        if (node.arguments.size() < N_start + N) {
            throw_renderer_error("function needs " + std::to_string(N_start + N) + " variables, but has only found " + std::to_string(node.arguments.size()), node);
        }

        for (size_t i = N_start; i < N_start + N; i += 1) {
            node.arguments[i]->accept(*this);
        }

        if (data_eval_stack.size() < N) {
            throw_renderer_error("function needs " + std::to_string(N) + " variables, but has only found " + std::to_string(data_eval_stack.size()), node);
        }

        std::array<const DataObject*, N> result;
        for (size_t i = 0; i < N; i += 1) {
            result[N - i - 1] = data_eval_stack.top();
            data_eval_stack.pop();

            if (!result[N - i - 1]) {
                const auto data_node = not_found_stack.top();
                not_found_stack.pop();

                if (throw_not_found) {
                    throw_renderer_error("variable '" + static_cast<std::string>(data_node->name) + "' not found", *data_node);
                }
            }
        }
        return result;
    }

    template <bool throw_not_found = true>
    Arguments get_argument_vector(const FunctionNode& node) {
        const size_t N = node.arguments.size();
        for (const auto& a : node.arguments) {
            a->accept(*this);
        }

        if (data_eval_stack.size() < N) {
            throw_renderer_error("function needs " + std::to_string(N) + " variables, but has only found " + std::to_string(data_eval_stack.size()), node);
        }

        Arguments result {N};
        for (size_t i = 0; i < N; i += 1) {
            result[N - i - 1] = data_eval_stack.top();
            data_eval_stack.pop();

            if (!result[N - i - 1]) {
                const auto data_node = not_found_stack.top();
                not_found_stack.pop();

                if (throw_not_found) {
                    throw_renderer_error("variable '" + static_cast<std::string>(data_node->name) + "' not found", *data_node);
                }
            }
        }
        return result;
    }

    void visit(const BlockNode& node) override;

    void visit(const TextNode& node) override;

    void visit(const ExpressionNode&) override {}

    void visit(const ArrayNode& node) override;
    void visit(const ObjectNode& node) override;

    void visit(const LiteralNode& node) override;

    void visit(const DataNode& node) override;

    void visit(const FunctionNode& node) override;

    void visit(const ExpressionListNode& node) override;

    void visit(const StatementNode&) override;

    void visit(const ForStatementNode&) override;

    void visit(const ForArrayStatementNode &node) override;

    void visit(const ForObjectStatementNode &node) override;

    void visit(const IfStatementNode &node) override;

    void visit(const IncludeStatementNode &node) override;

    void visit(const ExtendsStatementNode &node) override;

    void visit(const BlockStatementNode &node) override;

    void visit(const SetStatementNode &node) override;

public:
    explicit Renderer(
        const RenderConfig &config,
        const TemplateStorage &template_storage,
        const FunctionStorage &function_storage
    );

    void render_to(
        std::ostream &os,
        const Template &tmpl,
        const DataObject &data,
        DataObject *loop_data = nullptr
    );
};

} // namespace iris
