#include "Assertions.hpp"

#include "fixtures/FunctionFixture.hpp"
#include "iris/exceptions.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

namespace {

TEST_CASE_METHOD(tests::FunctionFixture, "CallbackError should provide render context") {

    SECTION("Trivial example") {
        auto ex = REQUIRE_EXCEPTION(
            render(R"({{ throw("callback") }})"),
            iris::RenderError
        );

        REQUIRE(ex.location.column == 4); // 4 because we count from the throw() call (which is really rather neat)
        REQUIRE(ex.location.line == 1);
    }
    SECTION("Multiline example") {
        auto ex = REQUIRE_EXCEPTION(
            render(R"(This is a line of text

This is also a line of text
This is about 30 chars before {{ throw("callback") }})"),
            iris::RenderError
        );

        REQUIRE(ex.location.column == 34);
        REQUIRE(ex.location.line == 4);
    }
}

}
