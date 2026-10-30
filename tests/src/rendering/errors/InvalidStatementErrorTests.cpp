#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <iris/iris.hpp>

#include "fixtures/EnvironmentFixture.hpp"

TEST_CASE_METHOD(tests::EnvironmentFixture, "Invalid set statements should throw a comprehensible error message") {
    REQUIRE_THROWS_WITH(
        render(
            R"({% set x = [ "a" ] )",
            iris::Object {},
            "file.txt"
        ),
        "[iris.exception.parser_error] (at 1:20 in file.txt) expected statement, got '<eof>'"
    );
    REQUIRE_THROWS_WITH(
        render(
            R"({% set x = [ "a" )",
            iris::Object {}
        ),
        "[iris.exception.parser_error] (at 1:18 in <unspecified>) malformed expression (unterminated object or array)"
    );
}
