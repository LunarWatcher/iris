#pragma once

#include "catch2/catch_test_macros.hpp"
#include "iris/exceptions.hpp"
#include <iris/iris.hpp>

namespace tests {

struct FunctionFixture {
    iris::Environment env;

    FunctionFixture() {
        env.add_callback("add", 2, [](iris::Arguments& args) {
            REQUIRE(args.size() == 2);

            return iris::DataObject(
                args.at(0)->get<iris::Int>() +
                args.at(1)->get<iris::Int>()
            );
        });
        env.add_void_callback("throw", 1, [](const iris::Arguments& args) {
            const auto& type = args.at(0)->get<iris::String>();

            if (type == "callback") {
                throw iris::CallbackError("This is a CallbackError");
            }
        });
    }

    std::string render(
        const std::string& tmpl,
        const iris::DataObject& obj = iris::Object {}
    ) {
        return env.render(tmpl, obj);
    }
};

}
