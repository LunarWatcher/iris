#include "iris/exceptions.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <iris/iris.hpp>

TEST_CASE("Malformed arrays in set statements should throw a comprehensible error message", "[errors]") {
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = ["a", "b", "c" %}{{ arr.2 }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:29) malformed expression (unterminated object or array)"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = ["a", "b", "c", [ "it" ] %}{{ arr.2 }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:39) malformed expression (unterminated object or array)"
    );
}

TEST_CASE("Malformed objects in set statements should throw a comprehensible error message", "[errors]") {
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = { "a": "b" %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:25) malformed expression (unterminated object or array)"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = { "a": { "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:34) malformed expression (unterminated object or array)"
    );
}

TEST_CASE("Malformed key-value pairs", "[errors]") {
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = { "a": } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:21) final entry in array is not a key-value pair"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = { "a": , "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:21) invalid key-value pair in object"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set arr = { : "v", "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:16) unexpected ':': no key defined"
    );
}

