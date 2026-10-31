#include <catch2/catch_test_macros.hpp>

#include "fixtures/EnvironmentFixture.hpp"
#include "iris/data/DataObject.hpp"

namespace {

TEST_CASE_METHOD(tests:: EnvironmentFixture, "Test that set statements do not affect global state") {
    iris::DataObject obj = iris::Object {
        {"obj", iris::Object {
            { "array", iris::Array { "a", "b", "c" } },
            { "fox_noises", true }
        }}
    };

    auto requireBaseline = [&]() {
        std::string baselineTemplate = "{{ obj }}";
        std::string baselineExpected = R"({"fox_noises":true,"array":["a","b","c"]})";
        REQUIRE(render(baselineTemplate, obj) == baselineExpected);
    };

    REQUIRE_NOTHROW(requireBaseline());

    REQUIRE(obj.size() != 0);

    REQUIRE_NOTHROW(render("{% set test = 69 %}", obj));

    REQUIRE(obj.contains(iris::DataPointer("obj.array")));
    REQUIRE_FALSE(obj.contains("test"));

    REQUIRE_NOTHROW(requireBaseline());

    REQUIRE(render("{{ obj.fox_noises }}", obj) == "true");

    REQUIRE(obj.size() != 0);
    {
        INFO(obj.serialize());
        REQUIRE(
            render(
                "{{ obj.fox_noises }}{% set obj.fox_noises = false %}{{ obj.fox_noises }}",
                obj
            )
            ==
            "truefalse"
        );
        REQUIRE(
            render(
                "{{ obj.fox_noises }}{% set obj.fox_noises = \"string\" %}{{ obj.fox_noises }}",
                obj
            )
            ==
            "truestring"
        );
    }
    REQUIRE(
        obj.at(iris::DataPointer("obj.fox_noises")).get<bool>()
    );

    REQUIRE_NOTHROW(requireBaseline());
}

TEST_CASE("Test that DataObject isn't mutable across copies", "[DataObject]") {
    iris::DataObject head = iris::Object {
        { "k", "v" },
        { "nested", iris::Object {
            {"k", "meow"}
        }}
    };

    iris::DataObject& ref = head;
    iris::DataObject copy = head;

    REQUIRE(ref == head);
    REQUIRE(copy == head);

    SECTION("New variable") {
        ref["m"] = 69;
        REQUIRE(head["m"] == 69);
        REQUIRE(copy.contains("m") == false);
    }

    SECTION("Top-level variable modification") {
        ref["k"] = 69;
        REQUIRE(head["k"] == 69);
        REQUIRE(copy["k"] == "v");

        copy["k"] = "rawr";

        REQUIRE(copy["k"] == "rawr");
        REQUIRE(head["k"] == 69);
    }

    SECTION("Nested variable modification") {
        auto ptr = iris::DataPointer("nested.k");
        ref[ptr] = 69;
        REQUIRE(head[ptr] == 69);
        REQUIRE(copy[ptr] == "meow");
    }
    
}

}
