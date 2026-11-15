#include "catch2/matchers/catch_matchers.hpp"
#include "iris/environment.hpp"
#include "fixtures/EnvironmentFixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(tests::EnvironmentFixture, "Unused macros should not be rendered") {
    REQUIRE(
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
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
        iris::render(
            R"({{ myMacro("cat") }}
{%- macro myMacro(name, orders = "meow") -%}
Hello {{ name }}, {{ orders -}}
{%- endmacro -%})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:17 in <unspecified>) unknown function myMacro"
    );
}
