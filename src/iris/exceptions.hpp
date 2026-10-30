#pragma once

#include <cstddef>
#include <format>
#include <stack>
#include <stdexcept>
#include <string>

namespace iris {

struct SourceLocation {
    size_t line;
    size_t column;

    /**
     * The identifier or (usually) full path to the file being processed.
     * This is only defined for ErrorType == Render or ErrorType == Parse
     */
    std::string filePathOrIdentifier = "";

    /**
     * The include stack that led to this file. Does not include the current file
     *
     * This is only defined for ErrorType == Render. At parse time, include statements are not evaluated, so the parse
     * itself will always be independent of the include stack. A parse-invalid template is invalid regardless of how
     * it's included.
     */
    std::stack<SourceLocation> importStack = {};
};

enum class ErrorType {
    Render,
    Parse,
    General
};

struct IrisError : public std::runtime_error {
    /**
     * Generic exception type. Note that this does not indicate where the error comes from, just a broad category of
     * errors.
     */
    const std::string type;
    const std::string message;

    const SourceLocation location;

    /**
     * Represents whether this error is from the parser, renderer, or outside those two.
     */
    const ErrorType errType;
    
    explicit IrisError(
        const std::string& type,
        const std::string& message,
        ErrorType errType
    )
        : std::runtime_error("[iris.exception." + type + "] " + message),
          type(type),
          message(message),
          location({0, 0}),
          errType(errType)
    {}

    explicit IrisError(
        const std::string& type,
        const std::string& message,
        ErrorType errType,
        SourceLocation&& location
    )
        : std::runtime_error(
            std::format(
                "[iris.exception.{}] (at {}:{} in {}) {}",
                type,
                location.line,
                location.column,
                location.filePathOrIdentifier == "" ? "<unspecified>" : location.filePathOrIdentifier,
                message
            )
        ),
          type(type),
          message(message),
          location(std::move(location)),
          errType(errType)
    {}
};

struct ContextlessRenderError : public IrisError {
    explicit ContextlessRenderError(
        const std::string& type,
        const std::string& message
    ) : IrisError(type, message, ErrorType::Render) {}
};

/**
 * Throw this exception to associate an error in a callback with error information related to the calling iris source
 * code.
 */
struct CallbackError : public ContextlessRenderError {
    explicit CallbackError(
        const std::string& message
    ) : ContextlessRenderError("callback", message) {}
};

struct ParserError : public IrisError {
    explicit ParserError(
        const std::string& message,
        SourceLocation&& location
    )
        : IrisError("parser_error", message, ErrorType::Parse, std::move(location)) {}
};

struct RenderError : public IrisError {
    explicit RenderError(
        const std::string& message,
        SourceLocation&& location
    ): IrisError("render_error", message, ErrorType::Render, std::move(location)) {}
};

struct FileError : public IrisError {
    explicit FileError(const std::string& message): IrisError("file_error", message, ErrorType::General) {}
    explicit FileError(const std::string& message, SourceLocation&& location)
        : IrisError("file_error", message, ErrorType::General, std::move(location)) {}
};

struct DataError : public IrisError {
    explicit DataError(
        const std::string& message
    ): IrisError("data_error", message, ErrorType::General) {}
};

} // namespace iris
