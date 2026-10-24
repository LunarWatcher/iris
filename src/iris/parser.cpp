#include "parser.hpp"
#include <memory>

namespace iris {


void Parser::pushExpressionNode(
    Arguments& args,
    std::shared_ptr<ExpressionNode>&& node,
    bool valueCanBeKey
) {
    if (objectStack.empty()) {
        args.emplace_back(node);
    } else {
        auto& parentNode = objectStack.top();
        if (std::holds_alternative<std::shared_ptr<ArrayNode>>(parentNode)) {
            std::get<std::shared_ptr<ArrayNode>>(parentNode)->children.push_back(
                std::move(node)
            );
        } else {
            if (objectsInObjectStack > objectKeys.size()) {
                if (
                    auto litNode = std::dynamic_pointer_cast<LiteralNode>(node);
                    valueCanBeKey && (
                        // TODO: allow keys to be variables
                        // (This doesn't work as long as DataObject only allows string keys)
                        litNode->value.is_string()
                    )
                ) {
                    objectKeys.push(litNode->value.serialize(false));
                    return;
                }
                throw_parser_error("expected string key");
            }
            auto k = objectKeys.top();
            objectKeys.pop();
            std::get<std::shared_ptr<ObjectNode>>(parentNode)->children.insert(
                {std::move(k), std::move(node)}
            );
        }
    }
}

}
