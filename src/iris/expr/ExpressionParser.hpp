#pragma once

#include "iris/data/DataObject.hpp"
#include <string_view>

namespace iris {

enum class ExpressionToken {
    Identify,
    Object,
    Array,
    Null,
    String,
    // We figure out if it's a float Later:tm:
    Number,
    Key,
};

class ExpressionParser {
private:
    std::string_view source;

    DataObject internalParse(ExpressionToken t);
public:
    ExpressionParser(std::string_view source) : source(source) {}

    DataObject parse();
};

}
