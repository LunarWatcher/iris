#include <iris/iris.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

TEST_CASE("Variable substitution", "[variables]") {
    std::string result = iris::render("{{ name }}", {
        { "name", "iris" },
        { "unused", "not included" }
    });

    REQUIRE(
        result == "iris"
    );
}

}
