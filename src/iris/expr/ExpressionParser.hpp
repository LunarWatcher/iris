#pragma once

#include <string_view>
namespace iris {

class ExpressionParser {
private:
    std::string_view source;

public:
    ExpressionParser(std::string_view source) : source(source) {}
};

}
