#include <catch2/catch_test_macros.hpp>

#include <iris/iris.hpp>

namespace {

TEST_CASE("If statements") {
    SECTION("Should work with variables") {
        REQUIRE(
            iris::render(
                R"({% if name == otherName %}yes{% endif %})",
                iris::Object {
                    {"name", "iris"},
                    {"otherName", "iris"}
                }
            )
            ==
            "yes"
        );
    }
    SECTION("Should work with a variable and a literal") {
        REQUIRE(
            iris::render(
                R"({% if name == "iris" %}yes{% endif %})",
                iris::Object {
                    {"name", "iris"},
                }
            )
            ==
            "yes"
        );
    }
}

}
