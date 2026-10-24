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

}
