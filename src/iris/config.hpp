#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "template.hpp"

namespace iris {

/*!
 * \brief Class for lexer configuration.
 */
struct LexerConfig {
    std::string statement_open {"{%"};
    std::string statement_open_no_lstrip {"{%+"};
    std::string statement_open_force_lstrip {"{%-"};
    std::string statement_close {"%}"};
    std::string statement_close_force_rstrip {"-%}"};
    std::string line_statement {"##"};
    std::string expression_open {"{{"};
    std::string expression_open_force_lstrip {"{{-"};
    std::string expression_close {"}}"};
    std::string expression_close_force_rstrip {"-}}"};
    std::string comment_open {"{#"};
    std::string comment_open_force_lstrip {"{#-"};
    std::string comment_close {"#}"};
    std::string comment_close_force_rstrip {"-#}"};
    std::string open_chars {"#{"};

    bool trim_blocks {false};
    bool lstrip_blocks {false};

    void update_open_chars();
};

/*!
 * \brief Class for parser configuration.
 */
struct ParserConfig {
    bool search_included_templates_in_files {true};

    std::function<Template(const std::filesystem::path&, const std::string&)> include_callback;
};

/*!
 * \brief Class for render configuration.
 */
struct RenderConfig {
    bool throw_at_missing_includes {true};
    bool html_autoescape {false};
};

} // namespace iris

