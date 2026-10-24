#include <catch2/catch_test_macros.hpp>

#include "fixtures/RenderFixture.hpp"

namespace {

TEST_CASE_METHOD(tests::RenderFixture, "add(a, b)", "[math]") {
    tmpl = "{{ a + b }}";

    SECTION("Signed + signed") {
        REQUIRE(
            renderArithmetic2(1, 2)
            == "3"
        );
        REQUIRE(
            renderArithmetic2(-1, 2)
            == "1"
        );
        REQUIRE(
            renderArithmetic2(1, -2)
            == "-1"
        );
    }
}

}
