#include <catch2/catch_test_macros.hpp>

#include "fixtures/EnvironmentFixture.hpp"

TEST_CASE_METHOD(tests::EnvironmentFixture, "The append() builtin") {
    REQUIRE(
        render(R"({%- set list = [] -%}
{{ list }}
{{- append(list, "owo") -}}
{{ list }})")
        ==
        R"([]
["owo"])"
    );
    // UB probe
    REQUIRE(
        render(R"({{- append(["hewwo"], "owo") -}})")
        ==
        ""
    );
}
