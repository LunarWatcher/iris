#include "renderer.hpp"
#include "iris/data/DataObject.hpp"

namespace iris {


void Renderer::visit(const ArrayNode& node) {
    DataObject object = Array {};

    for (auto& child : node.children) {
        
    }
}

void Renderer::visit(const ObjectNode& node) {
    DataObject object = Object {};
    
    for (auto& [k, v] : node.children) {
        
    }
}

}
