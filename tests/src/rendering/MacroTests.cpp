#include "catch2/matchers/catch_matchers.hpp"
#include "fixtures/FunctionFixture.hpp"
#include "iris/environment.hpp"
#include "fixtures/EnvironmentFixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(tests::EnvironmentFixture, "Unused macros should not be rendered") {
    REQUIRE(
        render(
            R"({%- macro myMacro() -%}
hello world
{%- endmacro -%}
potato)",
            iris::Object {}
        )
        ==
        "potato"
    );
    REQUIRE(
        render(
            R"({%- macro myMacro(name) -%}
hello {{ name }}
{%- endmacro -%}
potato)",
            iris::Object {}
        )
        ==
        "potato"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro statements without arguments") {
    REQUIRE(
        render(
            R"({%- macro myMacro() -%}
hello world
{%- endmacro -%}
{{- myMacro() -}})",
            iris::Object {}
        )
        ==
        "hello world\n"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro statements with arguments") {
    REQUIRE(
        render(
            R"({%- macro myMacro(name) -%}
hello {{ name -}}
{%- endmacro -%}
{{- myMacro(false) -}})",
            iris::Object {}
        )
        ==
        "hello false"
    );
    REQUIRE(
        render(
            R"({%- macro myMacro(name) -%}
hello {{ name -}}
{%- endmacro -%}
{{- myMacro("Zoe") -}})",
            iris::Object {}
        )
        ==
        "hello Zoe"
    );
    REQUIRE(
        render(
            R"({%- macro myMacro(name) -%}
hello {{ name -}}
{%- endmacro -%}
{{- myMacro({ "trans rights": "human rights" }) -}})",
            iris::Object {}
        )
        ==
        R"(hello {"trans rights":"human rights"})"
    );
    REQUIRE(
        render(
            R"({%- macro myMacro(name) -%}
hello {{ name -}}
{%- endmacro -%}
{{- myMacro({ "trans rights": trans_rights }) -}})",
            iris::Object {
                {"trans_rights", "human rights"}
            }
        )
        ==
        R"(hello {"trans rights":"human rights"})"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro statements with default arguments") {
    REQUIRE(
        render(
            R"({%- macro myMacro(name, orders = "meow") -%}
Hello {{ name }}, {{ orders -}}
{%- endmacro -%}
{{- myMacro("cat") -}})",
            iris::Object {}
        )
        ==
        "Hello cat, meow"
    );
    REQUIRE(
        render(
            R"({%- macro myMacro(name, orders = "meow") -%}
Hello {{ name }}, {{ orders -}}
{%- endmacro -%}
{{- myMacro("cat", "woof :3") -}})",
            iris::Object {}
        )
        ==
        "Hello cat, woof :3"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro declarations are order  dependent") {
    REQUIRE_THROWS_WITH(
        render(
            R"({{ myMacro("cat") }}
{%- macro myMacro(name, orders = "meow") -%}
Hello {{ name }}, {{ orders -}}
{%- endmacro -%})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:4 in <unspecified>) unknown function myMacro"
    );
}

TEST_CASE_METHOD(tests::FunctionFixture, "Types should be preserved") {
    struct Data {
        iris::Int a;
        std::optional<iris::Int> b;
        iris::Int result;
    };

    std::vector<Data> cases = {
        // Implicit 3
        { 0, std::nullopt, 3 },
        { 3, std::nullopt, 6 },
        { -3, std::nullopt, 0 },

        // Explicit 3
        { 0, 3, 3 },
        { 3, 3, 6 },
        { -3, 3, 0 },

        // explicit non-default-equivalent
        { 0, 6, 6 },
        { 3, 6, 9 },
        { -3, 6, 3 },
    };
    for (auto [a, b, expected] : cases) {
        iris::Object obj {
            {"a", a}
        };
        if (b) {
            obj["b"] = *b;
        }
        INFO(
            "a=" << a
            << ", b=" << (b ? std::to_string(*b) : "<undefined>")
            << ", expected=" << expected
        );
        REQUIRE(
            render(
                R"({%- macro customAdd(a, b = 3) -%}
{{ isNumber(a) }}
{{ isNumber(b) }}
{{ add(a, b) }}
{{ a + b }}
{%- endmacro -%}
{%- if exists("b") -%}
b: true
{{- customAdd(a, b) -}}
{%- else -%}
b: false
{{- customAdd(a) -}}
{%- endif -%})",
                obj
            )
            ==
            std::format(
                R"(b: {0}
true
true
{1}
{1}
)",
                b.has_value(),
                expected
            )
        );
    }
}
