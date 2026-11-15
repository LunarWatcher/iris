#include "function_storage.hpp"

namespace iris {

void FunctionStorage::add_builtin(std::string_view name, int num_args, Operation op) {
    function_storage.emplace(std::make_pair(static_cast<std::string>(name), num_args), FunctionData {op});
}

void FunctionStorage::add_callback(std::string_view name, int num_args, const CallbackFunction& callback) {
    function_storage.emplace(std::make_pair(static_cast<std::string>(name), num_args), FunctionData {Operation::Callback, callback});
}
void FunctionStorage::add_global_macro(
    std::string_view name,
    const std::shared_ptr<MacroStatementNode>& macroNode
) {
    if (macroNode == nullptr) {
        throw iris::DataError(
            "add_global_macro got null macroNode"
        );
    }
    // TODO: this technically allows for collisions with builtins, but there's no convenient way to check if a name
    // exists without knowing the number of parameters.
    // The function_storage itself should be altered to not use a pair. This also allows us to use the more performant
    // std::unordered_map.
    function_storage.emplace(
        std::make_pair(static_cast<std::string>(name), VARIADIC),
        FunctionData {macroNode}
    );
}

FunctionStorage::FunctionData FunctionStorage::find_function(std::string_view name, int num_args) const {
    auto it = function_storage.find(std::make_pair(static_cast<std::string>(name), num_args));
    if (it != function_storage.end()) {
        return it->second;
    }

    // Find variadic function
    it = function_storage.find(std::make_pair(static_cast<std::string>(name), VARIADIC));
    if (it != function_storage.end()) {
        return it->second;
    }

    return FunctionData {Operation::None};
}

}
