#pragma once

#include <filesystem>
#include <memory>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

#include "config.hpp"
#include "function_storage.hpp"
#include "lexer.hpp"
#include "node.hpp"
#include "template.hpp"
#include "token.hpp"

namespace iris {

/*!
 * \brief Class for parsing an iris Template.
 */
class Parser {
private:
    using Arguments = std::vector<std::shared_ptr<ExpressionNode>>;
    using OperatorStack = std::stack<std::shared_ptr<FunctionNode>>;
    using ObjectStack = std::stack<std::variant<std::shared_ptr<ObjectNode>, std::shared_ptr<ArrayNode>>>;

    enum class LiteralBlockType { Array, Object };

    const ParserConfig& config;

    static inline thread_local Template* currTemplate = nullptr;

    Lexer lexer;
    TemplateStorage& template_storage;
    const FunctionStorage& function_storage;

    Token tok, peek_tok;
    bool have_peek_tok {false};

    std::string_view literal_start;

    BlockNode* current_block {nullptr};
    ExpressionListNode* current_expression_list {nullptr};

    std::stack<IfStatementNode*> if_statement_stack;
    std::stack<ForStatementNode*> for_statement_stack;
    std::stack<BlockStatementNode*> block_statement_stack;

    void throw_parser_error(const std::string &message) const;

    void get_next_token();

    void get_peek_token();

    std::string_view getDataText();

    void add_operator(Arguments &arguments, OperatorStack& operator_stack);
    void processOperators(Arguments& arguments, std::stack<OperatorStack>& operatorStacks);

    void add_to_template_storage(const std::filesystem::path &path, std::string &template_name);

    std::string parse_filename() const;

    bool parse_expression(Template &tmpl, Token::Kind closing);

    void pushExpressionNode(
        Arguments& args,
        std::shared_ptr<ExpressionNode>&& node,
        std::stack<LiteralBlockType>& objects
    );

    std::shared_ptr<ExpressionNode> parse_expression(Template &tmpl);

    bool parse_statement(Template &tmpl, Token::Kind closing,
        const std::filesystem::path &path);

    void parse_into(Template &tmpl, const std::filesystem::path &path);

public:
    explicit Parser(
        const ParserConfig &parser_config,
        const LexerConfig &lexer_config,
        TemplateStorage &template_storage,
        const FunctionStorage &function_storage
    );

    Template parse(std::string_view input, const std::filesystem::path &path);

    void parse_into_template(
        Template &tmpl,
        const std::filesystem::path &filename
    );

    static std::string load_file(const std::filesystem::path &filename);
};

} // namespace iris
