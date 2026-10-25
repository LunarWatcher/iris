#include "iris/environment.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("For loops", "[for-loop]") {
    SECTION("Should expose a .index and .index1") {
        auto result = iris::render(
            R"({%- for i in range(4) -%}
{{ loop.index}}, {{ loop.index1 }}
{%- endfor -%})",
            iris::Object {}
        );
        REQUIRE(result == R"(0, 1
1, 2
2, 3
3, 4
)");
    }
    SECTION("Should allow access to nested for loops") {
        auto result = iris::render(
            R"({%- for k in range(3) -%}
{%- for v in range(3) -%}
{{k}} {{v}} {{ loop.index }} {{ loop.parent.index }}
{%- endfor -%}
{%- endfor -%})",
            iris::Object {}
        );

        REQUIRE(
            result == R"(0 0 0 0
0 1 1 0
0 2 2 0
1 0 0 1
1 1 1 1
1 2 2 1
2 0 0 2
2 1 1 2
2 2 2 2
)"
        );
    }
}
