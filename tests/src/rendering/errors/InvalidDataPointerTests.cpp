#include "iris/data/DataObject.hpp"
#include <format>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "fixtures/EnvironmentFixture.hpp"

namespace {

TEST_CASE_METHOD(tests::EnvironmentFixture, "Invalid array access should give usable error messages") {
    iris::DataObject data = iris::Object {
        {"k", {"a", "b"}}
    };

    REQUIRE(render("{{ k.1 }}", data) == "b");

    for (const std::string& identifier : std::vector<std::string> { "", "test.txt", "/tmp/trans-rights" }) {
        auto outId = identifier == "" ? "<unspecified>" : identifier;
        REQUIRE_THROWS_WITH(
            render(
                "{{ k.2 }}",
                data,
                identifier
            ),
            std::format(
                "[iris.exception.render_error] (at 1:4 in {}) variable 'k.2' not found",
                outId
            )
        );
        REQUIRE_THROWS_WITH(
            render(
                "{{ k.string }}",
                data,
                identifier
            ),
            std::format(
                "[iris.exception.render_error] (at 1:4 in {}) variable 'k.string' not found",
                outId
            )
        );
    }
}

}
