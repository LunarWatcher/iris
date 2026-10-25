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

        SECTION("Integers") {
            std::string result = iris::render(
                R"({% set x = 1234 %}{{ x }}, {{ isInteger(x) }}, {{ isFloat(x) }}, {{ isNumber(x) }})",
                iris::Object {}
            );
            REQUIRE(result == "1234, true, false, true");
        }

        SECTION("Doubles") {
            std::string result = iris::render(
                R"({% set x = 10.3 %}{{ x }}, {{ isInteger(x) }}, {{ isFloat(x) }}, {{ isNumber(x) }})",
                iris::Object {}
            );
            REQUIRE(result == "10.3, false, true, true");
        }
    }

    SECTION("Object statements") {
        SECTION("Basic") {
            std::string result = iris::render(
                R"({% set x = { "k": "value" } %}{{ x }}<>{{ x.k }})",
                iris::Object {}
            );
            REQUIRE(result == R"({"k":"value"}<>value)");
        }
        SECTION("With references") {
            std::string result = iris::render(
                R"({% set x = { "k": val } %}{{ x }}<>{{ x.k }})",
                iris::Object {
                    {"val", "value"}
                }
            );
            REQUIRE(result == R"({"k":"value"}<>value)");
        }
        SECTION("With mafs") {
            auto result = iris::render(
                R"({% set x = { "k": 1 + 2 } %}{{ x }}<>{{ x.k }}<>{{ isNumber(x.k) }})",
                iris::Object {}
            );
            REQUIRE(result == R"({"k":3}<>3<>true)");
        }
    }
    SECTION("Array statements") {
        SECTION("Basic") {
            std::string result = iris::render(
                R"({% set x = [ "k", "v" ] %}{{ x }}<>{{ x.1 }})",
                iris::Object {}
            );
            REQUIRE(result == R"(["k","v"]<>v)");
        }
        SECTION("With references") {
            std::string result = iris::render(
                R"({% set x = [ "k", v ] %}{{ x }}<>{{ x.1 }})",
                iris::Object {{"v", "mrrp meow :3"}}
            );
            REQUIRE(result == R"(["k","mrrp meow :3"]<>mrrp meow :3)");
        }
    }
}

}
