#include "environment.hpp"

namespace iris {

void Environment::set_statement(const std::string& open, const std::string& close) {
    lexer_config.statement_open = open;
    lexer_config.statement_open_no_lstrip = open + "+";
    lexer_config.statement_open_force_lstrip = open + "-";
    lexer_config.statement_close = close;
    lexer_config.statement_close_force_rstrip = "-" + close;
    lexer_config.update_open_chars();
}

void Environment::set_line_statement(const std::string& open) {
    lexer_config.line_statement = open;
    lexer_config.update_open_chars();
}

void Environment::set_expression(const std::string& open, const std::string& close) {
    lexer_config.expression_open = open;
    lexer_config.expression_open_force_lstrip = open + "-";
    lexer_config.expression_close = close;
    lexer_config.expression_close_force_rstrip = "-" + close;
    lexer_config.update_open_chars();
}

void Environment::set_comment(const std::string& open, const std::string& close) {
    lexer_config.comment_open = open;
    lexer_config.comment_open_force_lstrip = open + "-";
    lexer_config.comment_close = close;
    lexer_config.comment_close_force_rstrip = "-" + close;
    lexer_config.update_open_chars();
}

void Environment::set_trim_blocks(bool trim_blocks) {
    lexer_config.trim_blocks = trim_blocks;
}

void Environment::set_lstrip_blocks(bool lstrip_blocks) {
    lexer_config.lstrip_blocks = lstrip_blocks;
}

void Environment::set_search_included_templates_in_files(bool search_in_files) {
    parser_config.search_included_templates_in_files = search_in_files;
}

void Environment::set_throw_at_missing_includes(bool will_throw) {
    render_config.throw_at_missing_includes = will_throw;
}

void Environment::set_html_autoescape(bool will_escape) {
    render_config.html_autoescape = will_escape;
}

Template Environment::parse(std::string_view input, const std::string& filePathOrIdentifier) {
    Parser parser(parser_config, lexer_config, template_storage, function_storage);
    return parser.parse(
        input,
        filePathOrIdentifier
    );
}

Template Environment::parse_file(const std::filesystem::path& filename) {
    Parser parser(parser_config, lexer_config, template_storage, function_storage);
    auto result = Template(
        Parser::load_file(input_path / filename),
        input_path / filename
    );
    parser.parse_into_template(result, (input_path / filename).string());
    return result;
}

std::string Environment::render(
    std::string_view input,
    const DataObject& data,
    void* udata,
    const std::string& filePathOrIdentifier
) {
    if (!data.is_object()) {
        throw DataError(
            "The root data object must be an object!"
        );
    }
    return render(parse(input, filePathOrIdentifier), data, udata);
}

std::string Environment::render(
    const Template& tmpl,
    const DataObject& data,
    void* udata
) {
    if (!data.is_object()) {
        throw DataError(
            "The root data object must be an object!"
        );
    }
    std::stringstream os;
    render_to(os, tmpl, data, udata);
    return os.str();
}

std::string Environment::render_file(
    const std::filesystem::path& filename,
    const DataObject& data,
    void* udata
) {
    if (!data.is_object()) {
        throw DataError(
            "The root data object must be an object!"
        );
    }
    return render(parse_file(filename), data, udata);
}

void Environment::write(const std::filesystem::path& filename, const DataObject& data, const std::string& filename_out) {
    std::ofstream file(output_path / filename_out);
    file << render_file(filename, data);
    file.close();
}

void Environment::write(const Template& temp, const DataObject& data, const std::string& filename_out) {
    std::ofstream file(output_path / filename_out);
    file << render(temp, data);
    file.close();
}

std::ostream& Environment::render_to(
    std::ostream& os,
    const Template& tmpl,
    const DataObject& data,
    void* udata
) {
    if (!data.is_object()) {
        throw DataError(
            "The root data object must be an object!"
        );
    }
    Renderer renderer(
        render_config,
        template_storage,
        function_storage,
        udata
    );
    renderer.render_to(os, tmpl, std::stack<SourceLocation>{}, data);
    return os;
}

std::ostream& Environment::render_to(
    std::ostream& os,
    const std::string_view input,
    const DataObject& data,
    void* udata,
    const std::string& filePathOrIdentifier
) {
    return render_to(os, parse(input, filePathOrIdentifier), data, udata);
}

std::string Environment::load_file(const std::string& filename) {
    const Parser parser(
        parser_config,
        lexer_config,
        template_storage,
        function_storage
    );
    return Parser::load_file(input_path / filename);
}

void Environment::add_callback(const std::string& name, const CallbackFunction& callback) {
    add_callback(name, -1, callback);
}

void Environment::add_void_callback(const std::string& name, const VoidCallbackFunction& callback) {
    add_void_callback(name, -1, callback);
}

void Environment::add_callback(const std::string& name, int num_args, const CallbackFunction& callback) {
    function_storage.add_callback(name, num_args, callback);
}

void Environment::add_void_callback(const std::string& name, int num_args, const VoidCallbackFunction& callback) {
    function_storage.add_callback(name, num_args, [callback](Arguments& args, void* udata) {
        callback(args, udata);
        return DataObject();
    });
}

void Environment::add_callback(const std::string& name, const NonUDataCallbackFunction& callback) {
    add_callback(name, -1, callback);
}
void Environment::add_callback(const std::string& name, int num_args, const NonUDataCallbackFunction& callback) {
    function_storage.add_callback(name, num_args, [callback](Arguments& args, void*) {
        return callback(args);
    });
}

void Environment::add_void_callback(const std::string& name, const NonUDataVoidCallbackFunction& callback) {
    add_void_callback(name, -1, callback);
}

void Environment::add_void_callback(const std::string& name, int num_args, const NonUDataVoidCallbackFunction& callback) {
    function_storage.add_callback(name, num_args, [callback](Arguments& args, void*) {
        callback(args);
        return DataObject();
    });
}


void Environment::include_template(const std::string& name, const Template& tmpl) {
    template_storage[name] = tmpl;
}

void Environment::set_include_callback(const std::function<Template(const std::filesystem::path&, const std::string&)>& callback) {
    parser_config.include_callback = callback;
}

}

std::string iris::render(
    std::string_view input,
    const DataObject& data,
    const std::string& filePathOrIdentifier
) {
    return Environment().render(
        input,
        data,
        nullptr,
        filePathOrIdentifier
    );
}

void iris::render_to(std::ostream& os, std::string_view input, const DataObject& data) {
    Environment env;
    env.render_to(os, env.parse(input), data);
}

