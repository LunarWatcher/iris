#include <catch2/catch_test_macros.hpp>
#include <iris/iris.hpp>

namespace {

TEST_CASE("exists()") {
    iris::DataObject data = iris::Object {
        { "x", iris::Object {
                { "x", "mrrp meow :3" },
                { "foxgirl", "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" },
                {"nested", iris::Object {
                    {"nice", 69},
                }}
            }
        },
        { "y", iris::Array {
                "meow",
                "woof",
                "awoo"
            }
        },
    };
    SECTION("Global variables should be defined") {
        REQUIRE(
            iris::render(R"({{ exists("x") }})", data) == "true"
        );
        REQUIRE(
            iris::render(R"({{ exists("y") }})", data) == "true"
        );
    }
    SECTION("Object access should work") {
        REQUIRE(
            iris::render(R"({{ exists("x.x") }})", data) == "true"
        );
        REQUIRE(
            iris::render(R"({{ exists("x.foxgirl") }})", data) == "true"
        );
        REQUIRE(
            iris::render(R"({{ exists("x.nested.nice") }})", data) == "true"
        );
    }
    SECTION("Array access should work") {
        for (size_t i = 0; i < 3; ++i) {
            REQUIRE(
                iris::render(std::format(R"({{{{ exists("y.{}") }}}})", i), data) == "true"
            );
        }
    }
    SECTION("Non-existent keys should not throw") {
        REQUIRE(
            iris::render(R"({{ exists("z") }})", data) == "false"
        );
        REQUIRE(
            iris::render(R"({{ exists("z.a") }})", data) == "false"
        );
    }
    SECTION("Index-out-of-bounds in arrays should return false") {
        REQUIRE(
            iris::render(R"({{ exists("y.59") }})", data) == "false"
        );
    }
    SECTION("Non-existent and type-invalid keys should not throw") {
        REQUIRE(
            iris::render(R"({{ exists("x.59") }})", data) == "false"
        );
        REQUIRE(
            iris::render(R"({{ exists("y.y") }})", data) == "false"
        );
        REQUIRE(
            iris::render(R"({{ exists("59") }})", data) == "false"
        );
    }
    SECTION("Attempted access into non-existent intermediates should not throw") {
        REQUIRE(
            iris::render(R"({{ exists("a.b.c.d.e.f.g.65.h.1.i.j.k.l") }})", data) == "false"
        );
        REQUIRE(
            iris::render(R"({{ exists("a.69.c.d.e.f.g.65.h.1.i.j.k.l") }})", data) == "false"
        );
    }
}

}
