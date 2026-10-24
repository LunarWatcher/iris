#include "iris/exceptions.hpp"
#include <catch2/catch_test_macros.hpp>

#include <iris/iris.hpp>

TEST_CASE("Malformed arrays in set statements should throw a comprehensible error message") {
    try {
        auto result = iris::render(
            R"({% set arr = ["a", "b", "c" %}{{ arr.2 }})",
            iris::Object {}
        );
        FAIL("iris::render did not throw an exception, and instead returned " << result);
    } catch (iris::ParserError& err) {
        REQUIRE(
            std::string_view{
                err.what()
            } == "[iris.exception.parser_error] (at 1:29) malformed expression (unterminated object or array)"
        );
    }
}
