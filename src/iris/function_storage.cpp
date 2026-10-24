#include "function_storage.hpp"

namespace iris {

void FunctionStorage::add_builtin(std::string_view name, int num_args, Operation op) {
    function_storage.emplace(std::make_pair(static_cast<std::string>(name), num_args), FunctionData {op});
}

void FunctionStorage::add_callback(std::string_view name, int num_args, const CallbackFunction& callback) {
    function_storage.emplace(std::make_pair(static_cast<std::string>(name), num_args), FunctionData {Operation::Callback, callback});
}

FunctionStorage::FunctionData FunctionStorage::find_function(std::string_view name, int num_args) const {
    auto it = function_storage.find(std::make_pair(static_cast<std::string>(name), num_args));
    if (it != function_storage.end()) {
        return it->second;

        // Find variadic function
    } else if (num_args > 0) {
        it = function_storage.find(std::make_pair(static_cast<std::string>(name), VARIADIC));
        if (it != function_storage.end()) {
            return it->second;
        }
    }

    return FunctionData {Operation::None};
}

}
