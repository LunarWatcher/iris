#include "renderer.hpp"
#include "iris/data/DataObject.hpp"

namespace iris {


void Renderer::visit(const ArrayNode& node) {
    auto object = std::make_shared<DataObject>(
        Array {}
    );

    for (auto& child : node.children) {
        child->accept(*this);
        auto head = data_eval_stack.top();
        data_eval_stack.pop();

        object->push_back(*head);
    }
    data_tmp_stack.push_back(object);
    data_eval_stack.push(object.get());
}

void Renderer::visit(const ObjectNode& node) {
    auto object = std::make_shared<DataObject>(
        Object {}
    );

    for (auto& [k, v] : node.children) {
        v->accept(*this);

        auto head = data_eval_stack.top();
        data_eval_stack.pop();
        (*object)[k] = *head;
    }

    data_tmp_stack.push_back(object);
    data_eval_stack.push(object.get());
}

}
