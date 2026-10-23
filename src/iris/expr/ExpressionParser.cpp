#include "ExpressionParser.hpp"
#include <functional>

namespace iris {

DataObject internalParse(ExpressionToken token) {
    switch (token) {
    case ExpressionToken::Identify:
        break;
    case ExpressionToken::Object:
    case ExpressionToken::Array:
    case ExpressionToken::Null:
        return DataObject();
    case ExpressionToken::String:
    case ExpressionToken::Number:
    case ExpressionToken::Key:
      break;
    }
}

DataObject ExpressionParser::parse() {
    return internalParse(ExpressionToken::Identify);
}

}
