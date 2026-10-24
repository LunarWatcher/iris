#include "fixtures/FileFixture.hpp"
#include <catch2/catch_test_macros.hpp>

namespace {

TEST_CASE_METHOD(tests::FileFixture, "Environment", "[files]") {
    // C++26 is so full of cool fucking shit, including https://en.cppreference.com/cpp/preprocessor/embed, which would
    // make this test and similar future tests a lot easier to write
    REQUIRE(
        loadFile("block.txt")
        == R"({%- extends "base/basefile.txt" -%}
{%- block content -%}
trans rights are human rights
{%- endblock -%}
)");
}

TEST_CASE_METHOD(tests::FileFixture, "Block/extend statements", "[files]") {
    auto tmpl = parseFile("block.txt");

    std::string res = env.render(
        tmpl,
        iris::Object {}
    );

    REQUIRE(res == "trans rights are human rights\n");
}

TEST_CASE_METHOD(tests::FileFixture, "Include statements", "[files]") {
    SECTION("Should render out indentically to content.txt when {%- is used") {
        auto tmpl = parseFile("include.txt");
        auto res = env.render(
            tmpl,
            iris::Object {
                {"page", iris::Object {
                        {"content", "Did you know that mrrp meow? :3"}
                    }
                }
            });

        REQUIRE(res == R"(Hello world :3

Did you know that mrrp meow? :3
)"); // Loadbearing newline
    }
}

TEST_CASE_METHOD(tests::FileFixture, "content.txt should render three lines", "[files]") {
    auto tmpl = parseFile("content.txt");
    auto res = env.render(
        tmpl,
        iris::Object {
            {"page", iris::Object {
                    {"content", "Did you know that mrrp meow? :3"}
                }
            }
        });

    REQUIRE(res == R"(Hello world :3

Did you know that mrrp meow? :3
)"); // Loadbearing newline
}

TEST_CASE_METHOD(tests::FileFixture, "Include lookup functions hould be respected", "[files]") {
    env.set_include_callback([&](
        const std::filesystem::path& path,
        const std::string& tmplName
    ) {
        if (tmplName == "content.txt") {
            return parseFile(path / "block.txt");
        }
        REQUIRE(tmplName == "base/basefile.txt");
        // extends also uses this callback apparently. Good to know
        return parseFile(path / tmplName);
    });
    // Necessary to block the content.txt include from auto-resolving
    env.set_search_included_templates_in_files(false);

    auto tmpl = parseFile("include.txt");
    // The current include chain has no data in it, so this being empty is itself a good test
    auto res = env.render(tmpl, iris::Object {});

    REQUIRE(res == "trans rights are human rights\n");
}

}
