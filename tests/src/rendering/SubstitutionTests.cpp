#include <iris/iris.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

TEST_CASE("Variable substitution", "[variables]") {
    SECTION("Single variable") {
        std::string result = iris::render("{{ name }}", iris::Object {
                { "name", "iris" },
                { "unused", "not included" }
            });

        REQUIRE(
            result == "iris"
        );
    }

    SECTION("Object access") {
        std::string result = iris::render(
            "{{ names.iris }}",
            iris::Object {
                {"names", iris::Object {
                        {"iris", "Iris"}
                    }
                }
            }
        );
        REQUIRE(result == "Iris");
    }

    SECTION("Mixed object-array access") {
        std::string result = iris::render(
            "{{ names.0.name }}",
            iris::Object {
                {"names", {
                        iris::Object { {"name", "owo x3 rawr"}}
                }}
            }
        );
        REQUIRE(result == "owo x3 rawr");
    }
}

}
