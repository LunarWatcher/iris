#include <catch2/catch_test_macros.hpp>

#include "fixtures/FunctionFixture.hpp"

namespace {

TEST_CASE_METHOD(tests::FunctionFixture, "Test function calls") {
    REQUIRE(
        render("{{ add(1, 1) }}") == "2"
    );
    REQUIRE(
        render("{{ add(add(6, 9), 1) }}") == "16"
    );
    REQUIRE(
        render("{{ add(add(add(3, 3), 9), 1) }}") == "16"
    );
    REQUIRE(
        render("{{ add(add(add(1 + 2, 3), 9), 1) }}") == "16"
    );
}

TEST_CASE_METHOD(tests::FunctionFixture, "Test function calls in object literals") {
    REQUIRE(
        render(R"({% set x = { "v": add(3, 4) } %}{{ x }})")
        ==
        R"({"v":7})"
    );
    REQUIRE(
        render(R"({% set x = { "v": add(3, add(2, 2)) } %}{{ x }})")
        ==
        R"({"v":7})"
    );
    REQUIRE(
        render(R"({% set x = { "v": add(add(1, add(1, 1)), add(add(1, 1), add(2, add(0, 0)))) } %}{{ x }})")
        ==
        R"({"v":7})"
    );
}

}
