#include "fixtures/FunctionFixture.hpp"
#include <catch2/catch_test_macros.hpp>

namespace {

TEST_CASE_METHOD(tests::FunctionFixture, "Validate return types") {
    REQUIRE(
        render("{{ isNumber(add(1, 1)) }}") == "true"
    );
}

}
