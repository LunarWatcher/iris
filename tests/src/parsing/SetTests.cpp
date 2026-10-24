#include <iris/iris.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

TEST_CASE("Set statements", "[set]") {
    SECTION("Basic statements") {
        SECTION("String") {
            std::string result = iris::render(
                R"({% set x = "value" %}{{ x }})",
                iris::Object {}
            );
            REQUIRE(result == "value");
        }
        SECTION("Boolean") {
            std::string result = iris::render(
                R"({% set x = true %}{{ x }})",
                iris::Object {}
            );
            REQUIRE(result == "true");
            result = iris::render(
                R"({% set x = false %}{{ x }})",
                iris::Object {}
            );
            REQUIRE(result == "false");
        }
        SECTION("Null") {
            std::string result = iris::render(
                R"({% set x = null %}{{ x }})",
                iris::Object {}
            );
            INFO("{{ null }} should be an empty string");
            REQUIRE(result == "");
        }
    }

    SECTION("Object statements") {
        std::string result = iris::render(
            R"({% set x = { "k": "value" } %}{{ x }}<>{{ x.k }})",
            iris::Object {}
        );
        REQUIRE(result == R"({"k":"value"}<>value)");
    }
}

}
