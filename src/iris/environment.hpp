#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "config.hpp"
#include "function_storage.hpp"
#include "iris/data/DataObject.hpp"
#include "iris/exceptions.hpp"
#include "parser.hpp"
#include "renderer.hpp"
#include "template.hpp"

namespace iris {

using NonUDataCallbackFunction = std::function<DataObject(Arguments& args)>;
using NonUDataVoidCallbackFunction = std::function<void(Arguments& args)>;

/*!
 * \brief Class for changing the configuration.
 */
class Environment {
private:
    FunctionStorage function_storage;
    TemplateStorage template_storage;

protected:
    LexerConfig lexer_config;
    ParserConfig parser_config;
    RenderConfig render_config;

    std::filesystem::path input_path;
    std::filesystem::path output_path;

public:
    Environment()
        : Environment("") {}
    explicit Environment(const std::filesystem::path& global_path)
        : input_path(global_path),
          output_path(global_path) {}
    Environment(const std::filesystem::path& input_path, const std::filesystem::path& output_path): input_path(input_path), output_path(output_path) {}

    /// Sets the opener and closer for template statements
    void set_statement(const std::string& open, const std::string& close);

    /// Sets the opener for template line statements
    void set_line_statement(const std::string& open);

    /// Sets the opener and closer for template expressions
    void set_expression(const std::string& open, const std::string& close);

    /// Sets the opener and closer for template comments
    void set_comment(const std::string& open, const std::string& close);

    /// Sets whether to remove the first newline after a block
    void set_trim_blocks(bool trim_blocks);

    /// Sets whether to strip the spaces and tabs from the start of a line to a block
    void set_lstrip_blocks(bool lstrip_blocks);

    /// Sets the element notation syntax
    void set_search_included_templates_in_files(bool search_in_files);

    /// Sets whether a missing include will throw an error
    void set_throw_at_missing_includes(bool will_throw);

    /// Sets whether we'll automatically perform HTML escape
    void set_html_autoescape(bool will_escape);

    /**
     * \param input                 The template content
     * \param filePathOrIdentifier  An absolute path to the source file, or an optional identifier to more easily
     *                              identify the template in stacktraces
     */
    Template parse(
        std::string_view input,
        const std::string& filePathOrIdentifier = ""
    );

    Template parse_file(const std::filesystem::path& filename);

    std::string render(
        std::string_view input,
        const DataObject& data,
        void* udata = nullptr,
        const std::string& filePathOrIdentifier = ""
    );
    std::string render(
        const Template& tmpl,
        const DataObject& data,
        void* udata = nullptr
    );
    std::string render_file(
        const std::filesystem::path& filename,
        const DataObject& data,
        void* udata = nullptr
    );
    std::ostream& render_to(
        std::ostream& os,
        const Template& tmpl,
        const DataObject& data,
        void* udata = nullptr
    );
    std::ostream& render_to(
        std::ostream& os,
        const std::string_view input,
        const DataObject& data,
        void* udata = nullptr,
        const std::string& filePathOrIdentifier = ""
    );

    void write(const std::filesystem::path& filename, const DataObject& data, const std::string& filename_out);

    void write(const Template& temp, const DataObject& data, const std::string& filename_out);

    std::string load_file(const std::string& filename);


    // TODO: I tried and failed to turn all these into a template. As far as I can tell, C++ compilers may be too dumb
    // to match against an std::function<...> type. Using a raw C type didn't work either, it failed to link against
    // both vararg and hard-coded types.
    // There's 8 of these bastards though, it would be nice if we could collapse them a bit. With templates, we could in
    // theory also allow static_cast on the environment side. Or would that cause errors that are too non-transparent to
    // the developer?
    /*!
      @brief Adds a variadic callback
    */
    void add_callback(const std::string& name, const CallbackFunction& callback);

    /*!
      @brief Adds a callback with given number or arguments
    */
    void add_callback(const std::string& name, int num_args, const CallbackFunction& callback);

    /*!
      @brief Adds a variadic void callback
    */
    void add_void_callback(const std::string& name, const VoidCallbackFunction& callback);

    /*!
      @brief Adds a void callback with given number or arguments
    */
    void add_void_callback(const std::string& name, int num_args, const VoidCallbackFunction& callback);

    /*!
      @brief Adds a variadic callback
    */
    void add_callback(const std::string& name, const NonUDataCallbackFunction& callback);

    /*!
      @brief Adds a callback with given number or arguments
    */
    void add_callback(const std::string& name, int num_args, const NonUDataCallbackFunction& callback);

    /*!
      @brief Adds a variadic void callback
    */
    void add_void_callback(const std::string& name, const NonUDataVoidCallbackFunction& callback);

    /*!
      @brief Adds a void callback with given number or arguments
    */
    void add_void_callback(const std::string& name, int num_args, const NonUDataVoidCallbackFunction& callback);

    /** Includes a template with a given name into the environment.
     * Then, a template can be rendered in another template using the
     * include "<name>" syntax.
     */
    void include_template(const std::string& name, const Template& tmpl);

    /*!
      @brief Sets a function that is called when an included file is not found
    */
    void set_include_callback(const std::function<Template(const std::filesystem::path&, const std::string&)>& callback);
};

/*!
  @brief render with default settings to a string
*/
extern std::string render(
    std::string_view input,
    const DataObject& data,
    const std::string& filePathOrIdentifier = ""
);

/*!
  @brief render with default settings to the given output stream
*/
extern void render_to(std::ostream& os, std::string_view input, const DataObject& data);

} // namespace iris

