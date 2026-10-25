#include "iris/data/DataObject.hpp"
#include "iris/environment.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

namespace {

TEST_CASE("Invalid array access should give usable error messages") {
    iris::DataObject data = iris::Object {
        {"k", {"a", "b"}}
    };

    REQUIRE(iris::render("{{ k.1 }}", data) == "b");
    REQUIRE_THROWS_WITH(
        iris::render(
            "{{ k.2 }}",
            data
        ),
        "[iris.exception.render_error] (at 1:4) variable 'k.2' not found"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            "{{ k.string }}",
            data
        ),
        R"([iris.exception.render_error] (at 1:4) expected valid number, but found "string" while accessing path "k.string")"
    );
}

}
