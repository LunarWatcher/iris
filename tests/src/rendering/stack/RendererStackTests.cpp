#include <catch2/catch_test_macros.hpp>
#include "Assertions.hpp"
#include "fixtures/FunctionFixture.hpp"
#include "iris/exceptions.hpp"

namespace {

TEST_CASE_METHOD(tests::FunctionFixture, "Verify that the stack is currently parsed and identified in nested calls") {
    auto ex = REQUIRE_EXCEPTION(
        renderFile("chained-2.txt", iris::Object { { "throw", true } }),
        iris::RenderError
    );

    auto loc = ex.location;
    REQUIRE(loc.column == 4);
    REQUIRE(loc.line == 1);

    REQUIRE(
        loc.filePathOrIdentifier == resolvePath("base/throw.txt")
    );
    REQUIRE(loc.importStack.size() == 3);

    SECTION("Validate the import stack locations") {
        auto chained0 = loc.importStack.top();
        loc.importStack.pop();

        REQUIRE(
            chained0.filePathOrIdentifier == resolvePath("chained-0.txt")
        );
        REQUIRE(chained0.column == 16);
        REQUIRE(chained0.line == 2);

        auto chained1 = loc.importStack.top();
        loc.importStack.pop();

        REQUIRE(
            chained1.filePathOrIdentifier == resolvePath("chained-1.txt")
        );
        REQUIRE(chained1.column == 12);
        REQUIRE(chained1.line == 2);

        auto chained2 = loc.importStack.top();
        loc.importStack.pop();

        REQUIRE(
            chained2.filePathOrIdentifier == resolvePath("chained-2.txt")
        );
        REQUIRE(chained2.column == 19);
        REQUIRE(chained2.line == 2);

    }

    // This feels like a bad use of sections, but it feels much clearer in terms of intent
    SECTION(
        "Validate that none of the intermediate source locations have not been given stacks"
    ) {
        while (!loc.importStack.empty()) {
            auto it = loc.importStack.top();
            loc.importStack.pop();
            INFO(it.filePathOrIdentifier);

            REQUIRE(it.importStack.size() == 0);
        }
    }
}

}
