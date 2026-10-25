#pragma once

#include <string>
#include <iris/iris.hpp>

namespace tests {

/**
 * This fixture is used for rendering the same template (tmpl) with multiple sets of data.
 */
struct RenderFixture {
    std::string tmpl;

    std::string renderArithmetic2(iris::DataObject a, iris::DataObject b) {
        return iris::render(tmpl, iris::Object { {"a", a}, {"b", b} });
    }
};

}
