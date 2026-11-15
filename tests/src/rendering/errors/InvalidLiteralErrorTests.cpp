#include "fixtures/FunctionFixture.hpp"
#include "iris/exceptions.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "fixtures/EnvironmentFixture.hpp"

#include <iris/iris.hpp>

TEST_CASE_METHOD(tests::EnvironmentFixture, "Malformed arrays in set statements should throw a comprehensible error message", "[errors]") {
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = ["a", "b", "c" %}{{ arr.2 }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:29 in <unspecified>) malformed expression (unterminated object or array)"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = ["a", "b", "c", [ "it" ] %}{{ arr.2 }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:39 in <unspecified>) malformed expression (unterminated object or array)"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Malformed objects in set statements should throw a comprehensible error message", "[errors]") {
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = { "a": "b" %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:25 in <unspecified>) malformed expression (unterminated object or array)"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = { "a": { "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:34 in <unspecified>) malformed expression (unterminated object or array)"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Malformed key-value pairs", "[errors]") {
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = { "a": } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:21 in <unspecified>) final entry in array is not a key-value pair"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = { "a": , "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:21 in <unspecified>) invalid key-value pair in object"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({% set arr = { : "v", "b": "c" } %}{{ arr.a }})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:16 in <unspecified>) unexpected ':': no key defined"
    );
}

TEST_CASE_METHOD(tests::FunctionFixture, "Invalid variable access should produce a correct error", "[errors]") {
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": context.invalid,
    "key": "value"
} %}
)",
            iris::Object {
                {"context", iris::Object {}}
            }
        ),
        "[iris.exception.render_error] (at 3:13 in <unspecified>) Dereferenced invalid variable: Data pointer: context.invalid"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": nonexistent,
    "key": "value"
} %}
)",
            iris::Object {
                {"context", iris::Object {}}
            }
        ),
        "[iris.exception.render_error] (at 3:13 in <unspecified>) Dereferenced invalid variable: Data pointer: nonexistent"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": nonexistent.alsononexistent,
    "key": "value"
} %}
)",
            iris::Object {
                {"context", iris::Object {}}
            }
        ),
        "[iris.exception.render_error] (at 3:13 in <unspecified>) Dereferenced invalid variable: Data pointer: nonexistent.alsononexistent"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": invalid()),
    "key": "value"
} %}
)",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 3:13 in <unspecified>) unknown function invalid"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": add(1, nonexistent),
    "key": "value"
} %}
)",
            iris::Object {}
        ),
        "[iris.exception.render_error] (at 3:20 in <unspecified>) variable 'nonexistent' not found"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"(
{% set obj = {
    "key1": invalid(1, nonexistent),
    "key": "value"
} %}
)",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 3:13 in <unspecified>) unknown function invalid"
    );
}
