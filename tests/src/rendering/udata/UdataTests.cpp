#include "fixtures/EnvironmentFixture.hpp"
#include "iris/data/DataObject.hpp"
#include <catch2/catch_test_macros.hpp>

namespace {

struct TestData {
    std::string v;
};

TEST_CASE_METHOD(tests::EnvironmentFixture, "Test udata forwarding in all the things") {
    iris::DataObject obj = iris::Object {};

    TestData data {
        "mrrp meow"
    };

    env.add_void_callback("void", 1, [](iris::Arguments& args, void* udata) {
        REQUIRE(udata != nullptr);
        auto* d = (TestData*) udata;
        REQUIRE(args.at(0)->is_string());
        REQUIRE(args.at(0)->get<iris::String>() == d->v);
    });

    env.add_callback("return", -1, [](iris::Arguments& args, void* udata)  -> iris::DataObject {
        REQUIRE(udata != nullptr);
        auto* d = (TestData*) udata;
        if (args.size() == 1) {
            REQUIRE(args.at(0)->is_string());
            return args.at(0)->get<iris::String>() == d->v;
        } else {
            return d->v;
        }
    });

    std::string templateContent = loadFile("udata-tests.txt");

    REQUIRE(templateContent == R"({{ void("mrrp meow") }}
{{ return("mrrp meow") }}
{{ return("meow mrrp") }}
{{ return() }}
)");

    auto parsedTemplate = parseString(templateContent);

    const std::string expectedRender = R"(
true
false
mrrp meow
)"; // Loadbearing newline
    
    SECTION("render(str)") {
        std::string result = env.render(templateContent, iris::Object{}, &data);
        REQUIRE(result == expectedRender);
    }
    SECTION("render(template)") {
        std::string result = env.render(parsedTemplate, iris::Object{}, &data);
        REQUIRE(result == expectedRender);
    }
    SECTION("render_to(template)") {
        std::stringstream ss;
        env.render_to(ss, parsedTemplate, iris::Object{}, &data);
        REQUIRE(ss.str() == expectedRender);
    }
    SECTION("render_to(string)") {
        std::stringstream ss;
        env.render_to(ss, templateContent, iris::Object{}, &data);
        REQUIRE(ss.str() == expectedRender);
    }
    SECTION("render_file()") {
        auto result = env.render_file(
            resolvePath("udata-tests.txt"),
            iris::Object {},
            &data
        );
        REQUIRE(result == expectedRender);
    }
}

}
