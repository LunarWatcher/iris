#include <catch2/catch_test_macros.hpp>

#include "fixtures/EnvironmentFixture.hpp"

namespace {

TEST_CASE_METHOD(tests::EnvironmentFixture, "Verify that the parser forwards paths correctly") {
    auto tmpl = parse("{{ value }}", "index.md");
    REQUIRE(tmpl.pathOrIdentifier == "index.md");

    tmpl = parse("{{ value }}", "/usr/bin/bash");
    REQUIRE(tmpl.pathOrIdentifier == "/usr/bin/bash");
}

}
