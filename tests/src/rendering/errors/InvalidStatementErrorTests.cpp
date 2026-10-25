#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <iris/iris.hpp>

TEST_CASE("Invalid set statements should throw a comprehensible error message") {
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set x = [ "a" ] )",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:20) expected statement, got '<eof>'"
    );
    REQUIRE_THROWS_WITH(
        iris::render(
            R"({% set x = [ "a" )",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:18) malformed expression (unterminated object or array)"
    );
}
