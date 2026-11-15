#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "fixtures/FunctionFixture.hpp"
#include "iris/exceptions.hpp"

TEST_CASE_METHOD(tests::FunctionFixture, "Test exceptions on non-existing functions") {
    REQUIRE_THROWS_WITH(
        render("{{ hello() }}"),
        R"([iris.exception.parser_error] (at 1:4 in <unspecified>) unknown function hello)"
    );
}

TEST_CASE_METHOD(tests::FunctionFixture, "Test exceptions on non-existent function in a base extends call") {
    SECTION("Without udata, in include") {
        REQUIRE_THROWS_MATCHES(
            renderFile("invalid_func_in_extends.txt"),
            iris::ParserError,
            Catch::Matchers::Predicate<iris::ParserError>(
                [](const iris::ParserError& err) -> bool {
                    return err.message == "unknown function hello";
                }
            )
        );
    }
    SECTION("Without udata, after include") {
        REQUIRE_THROWS_MATCHES(
            renderFile("extend_invalid_func_after_include.txt"),
            iris::ParserError,
            Catch::Matchers::Predicate<iris::ParserError>(
                [](const iris::ParserError& err) -> bool {
                    return err.message == "unknown function hello";
                }
            )
        );
    }
    SECTION("With udata") {
        struct UData {
            std::string whatever = "unused";
        };
        UData ud;
        REQUIRE_THROWS_MATCHES(
            renderFileWithUData("invalid_func_in_extends.txt", &ud),
            iris::ParserError,
            Catch::Matchers::Predicate<iris::ParserError>(
                [](const iris::ParserError& err) -> bool {
                    return err.message == "unknown function hello";
                }
            )
        );
    }
}

TEST_CASE_METHOD(tests::FunctionFixture, "Test exceptions on incompatible function calls") {
    REQUIRE_THROWS_WITH(
        render("{{ add(1) }}"),
        R"([iris.exception.parser_error] (at 1:4 in <unspecified>) unknown function add)"
    );
    REQUIRE_THROWS_WITH(
        render("{{ add(1, 1, 1) }}"),
        R"([iris.exception.parser_error] (at 1:4 in <unspecified>) unknown function add)"
    );
}
