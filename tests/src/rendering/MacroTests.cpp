#include "catch2/matchers/catch_matchers.hpp"
#include "fixtures/FunctionFixture.hpp"
#include "iris/data/DataObject.hpp"
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

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro declarations are order dependent") {
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

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macro declarations may not be nested") {
    REQUIRE_THROWS_WITH(
        render(
            R"(
{%- macro myMacro(name, orders = "meow") -%}
    {%- macro myMacro(name, orders = "meow") -%}
    Hello {{ name }}, {{ orders -}}
    {%- endmacro -%}
{%- endmacro -%})",
            iris::Object {},
            // Might as well sprinkle in an explicit identifier every once in a while just for good measure
            "my template.iris"
        ),
        "[iris.exception.parser_error] (at 3:9 in my template.iris) macro declarations may not be nested"
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

TEST_CASE_METHOD(tests::EnvironmentFixture, "Macros should be possible to call from within a macro body") {
    REQUIRE(
        render(
            R"(
{%- macro myMacro(n) -%}
    {{ n }}
    {%- if n > 0 -%}
        {{- myMacro(n - 1) -}}
    {%- endif -%}
{%- endmacro -%}
{{- myMacro(10) -}})",
            iris::Object {}
        )
        ==
        R"(
10
9
8
7
6
5
4
3
2
1
0
)"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Default args may not be declared before non-defaulted args") {
    REQUIRE_THROWS_WITH(
        render(
            R"({%- macro myMacro(n, a = "colon three", c) -%}
{%- endmacro -%})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:42 in <unspecified>) Argument at position 2 defaulted its argument. All subsequent arguments must also be defaulted"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({%- macro myMacro(n, a = "colon three", c) -%}
{%- endmacro -%})",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:42 in <unspecified>) Argument at position 2 defaulted its argument. All subsequent arguments must also be defaulted"
    );
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "Invalid macro calls should give sane errors") {
    struct Data {
        std::string args;
        std::optional<int> errorActualArgs = std::nullopt;
    };

    std::vector<Data> cases = {
        Data { "" , 0 },
        Data { "1" , 1 },
        Data { "1, 2" , 2 },
        Data { "1, 2, 3" },
        Data { "1, 2, 3, 4" },
        Data { "1, 2, 3, 4, 5" },
        Data { "1, 2, 3, 4, 5, 6" },
        Data { "1, 2, 3, 4, 5, 6, 7", 7 },
        Data { "1, 2, 3, 4, 5, 6, 7, 8", 8 },
    };
    
    for (const auto& [args, errorActualArgs] : cases) {
        auto text =
            R"({%- macro myMacro(a, b, c, d = "d", e = 69, f = false) -%}success{%- endmacro -%})"
            + std::format(
                "\n{{{{ myMacro({}) }}}}",
                args
            );
        INFO("Template body: " << text);
        if (errorActualArgs) {
            REQUIRE_THROWS_WITH(
                render(text, iris::Object {}),
                std::format(
                    "[iris.exception.parser_error] (at 2:4 in <unspecified>) Macro myMacro expected at least 3 arguments and at most 6 arguments, but got {}",
                    *errorActualArgs
                )
            );
        } else {
            REQUIRE(
                render(text, iris::Object {})
                ==
                "success"
            );
        }
    }
}

TEST_CASE_METHOD(tests::EnvironmentFixture, "The environment should allow accepting global macros") {
    auto tmpl = parseFile(
        "macros/macro_defs.iris"
    );

    auto beforeSize = env.getFunctionStorage().getRawData().size();
    REQUIRE_NOTHROW(env.accept_macros(tmpl));
    auto afterSize = env.getFunctionStorage().getRawData().size();

    REQUIRE(afterSize == beforeSize + 1);

    REQUIRE(
        render("{{ meow() }}", iris::Object {})
        ==
        "meow meow meow meow meow meow\n"
    );
}
