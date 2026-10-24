#pragma once

#include <cstddef>
#include <string_view>

#include "config.hpp"
#include "exceptions.hpp"
#include "token.hpp"

namespace iris {

/*!
 * \brief Class for lexing an iris Template.
 */
class Lexer {
    enum class State {
        Text,
        ExpressionStart,
        ExpressionStartForceLstrip,
        ExpressionBody,
        LineStart,
        LineBody,
        StatementStart,
        StatementStartNoLstrip,
        StatementStartForceLstrip,
        StatementBody,
        CommentStart,
        CommentStartForceLstrip,
        CommentBody,
    };

    enum class MinusState {
        Operator,
        Number,
    };

    const LexerConfig& config;

    State state;
    MinusState minus_state;
    std::string_view m_in;
    size_t tok_start;
    size_t pos;

    Token scan_body(std::string_view close, Token::Kind closeKind, std::string_view close_trim = std::string_view(), bool trim = false);

    Token scan_id();

    Token scan_number();

    Token scan_string();

    Token make_token(Token::Kind kind) const;

    void skip_whitespaces_and_newlines();

    void skip_whitespaces_and_first_newline();

    static std::string_view clear_final_line_if_whitespace(std::string_view text);

public:
    explicit Lexer(const LexerConfig& config);

    SourceLocation current_position() const;

    void start(std::string_view input);

    Token scan();

    const LexerConfig& get_config() const;
};

} // namespace iris
