#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace iris {

struct SourceLocation {
    size_t line;
    size_t column;
};

struct IrisError : public std::runtime_error {
    const std::string type;
    const std::string message;

    const SourceLocation location;

    explicit IrisError(const std::string& type, const std::string& message)
        : std::runtime_error("[iris.exception." + type + "] " + message), type(type), message(message), location({0, 0}) {}

    explicit IrisError(const std::string& type, const std::string& message, SourceLocation location)
        : std::runtime_error("[iris.exception." + type + "] (at " + std::to_string(location.line) + ":" + std::to_string(location.column) + ") " + message),
          type(type), message(message), location(location) {}
};

struct ParserError : public IrisError {
    explicit ParserError(const std::string& message, SourceLocation location): IrisError("parser_error", message, location) {}
};

struct RenderError : public IrisError {
    explicit RenderError(const std::string& message, SourceLocation location): IrisError("render_error", message, location) {}
};

struct FileError : public IrisError {
    explicit FileError(const std::string& message): IrisError("file_error", message) {}
    explicit FileError(const std::string& message, SourceLocation location): IrisError("file_error", message, location) {}
};

struct DataError : public IrisError {
    explicit DataError(const std::string& message, SourceLocation location): IrisError("data_error", message, location) {}
};

} // namespace iris
