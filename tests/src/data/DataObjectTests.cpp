#include "iris/data/DataPointer.hpp"
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <iris/data/DataObject.hpp>

namespace {

TEST_CASE("DataObject init") {
    SECTION("Should be constructible with an int") {
        iris::DataObject o(2);
        REQUIRE(o.is_int());
        REQUIRE(o.get<iris::Int>() == 2);
    }

    SECTION("Should be constructible with an unsigned int") {
        iris::DataObject o((uint32_t) 2);
        REQUIRE(o.is_uint());
        REQUIRE(o.get<iris::UInt>() == 2);
    }

    SECTION("Should be constructable with a float") {
        iris::DataObject o(4.2);
        REQUIRE(o.is_float());
        REQUIRE(o.get<iris::Float>() == 4.2);
    }

    SECTION("Should be constructable with a bool") {
        iris::DataObject o(true);
        REQUIRE(o.is_bool());
        REQUIRE(o.get<bool>() == true);
    }

    SECTION("Should be constructable with a string") {
        iris::DataObject o("very creative string");
        REQUIRE(o.is_string());
        REQUIRE(o.get<iris::String>() == "very creative string");
    }

    SECTION("Should be constructable with a vector") {
        iris::DataObject o {
            "a", "b", "c", 69
        };
        REQUIRE(o.is_array());

        // Note to self: { std::vector(...) } results in a nested vector, and I think I'm going to classify this as
        // intentional for now. Would be nice if { std::vector } == ( std::vector ), but this is my first time using
        // intializer lists, so I'm fine with this quirk
        iris::DataObject o2 (
            iris::Array {"a", "b", "c", 69}
        );
        REQUIRE(o2.is_array());

        INFO(o);
        INFO(o2);
        REQUIRE(o == o2);
        REQUIRE(o.serialize() == R"(["a","b","c",69])");
    }

    SECTION("Should be constructable with an object") {
        SECTION("Basic object") {
            iris::DataObject o (
                iris::Object {
                    {"k", "v"},
                    {"k2", "v2"},
                }
            );
            REQUIRE(o.is_object());
        }
        SECTION("Involved object") {
            iris::DataObject o = iris::Object {
                { "k", "v" },
                { "k2", iris::Object {
                        {"nk1", "nv1"},
                        {"nk2", "nv2"},
                    }
                },
                { "k3", 69},
            };

            REQUIRE(o.is_object());
            REQUIRE(
                o.serialize()
                ==
                // TODO: is this order guaranteed to be stable?
                R"({"k3":69,"k2":{"nk2":"nv2","nk1":"nv1"},"k":"v"})"
            );
        }
    }
}

TEST_CASE("DataObject equality") {
    SECTION("Primitives") {
        REQUIRE(iris::DataObject(3) == iris::DataObject(3));
        INFO((iris::DataObject(3) == iris::DataObject(3.0)));
        REQUIRE(iris::DataObject(3) != iris::DataObject(3.0));
        REQUIRE(iris::DataObject(3.0) == iris::DataObject(3.0));
        REQUIRE(iris::DataObject(3) != iris::DataObject(4.0));
        REQUIRE(iris::DataObject("3.0") != iris::DataObject(3.0));
        REQUIRE(iris::DataObject("3.0") == iris::DataObject("3.0"));
    }
    SECTION("Null") {
        REQUIRE(iris::DataObject() == iris::DataObject(nullptr));
        REQUIRE(iris::DataObject() != iris::DataObject("3"));
        REQUIRE(iris::DataObject() != iris::DataObject(3));
        REQUIRE(iris::DataObject() != iris::DataObject(3.0));
    }
    SECTION("Vectors (single type)") {
        REQUIRE(iris::DataObject { 1, 2, 3 } == iris::DataObject { 1, 2, 3 });
    }
    SECTION("Vectors (mixed types") {
        REQUIRE(iris::DataObject { 1, "a" } == iris::DataObject { 1, "a" });
    }
}

TEST_CASE("DataObject comparison") {
    SECTION("greater than/less than") {
        SECTION("Ints") {
            REQUIRE(iris::DataObject(3) > iris::DataObject(2));
            REQUIRE(iris::DataObject(2) < iris::DataObject(3));
            REQUIRE_FALSE(iris::DataObject(3) < iris::DataObject(2));
            REQUIRE_FALSE(iris::DataObject(2) > iris::DataObject(3));

            REQUIRE_FALSE(iris::DataObject(3) > iris::DataObject(3));
            REQUIRE_FALSE(iris::DataObject(3) < iris::DataObject(3));
        }

        SECTION("Floats") {
            REQUIRE(iris::DataObject(3.0) > iris::DataObject(2.0));
            REQUIRE(iris::DataObject(2.0) < iris::DataObject(3.0));
            REQUIRE_FALSE(iris::DataObject(3.0) < iris::DataObject(2.0));
            REQUIRE_FALSE(iris::DataObject(2.0) > iris::DataObject(3.0));

            REQUIRE_FALSE(iris::DataObject(3.0) > iris::DataObject(3.0));
            REQUIRE_FALSE(iris::DataObject(3.0) < iris::DataObject(3.0));
        }
    }
    SECTION("greater than/less than or equal") {
        SECTION("Ints") {
            REQUIRE(iris::DataObject(3) >= iris::DataObject(2));
            REQUIRE(iris::DataObject(2) <= iris::DataObject(3));
            REQUIRE_FALSE(iris::DataObject(3) <= iris::DataObject(2));
            REQUIRE_FALSE(iris::DataObject(2) >= iris::DataObject(3));

            REQUIRE(iris::DataObject(3) >= iris::DataObject(3));
            REQUIRE(iris::DataObject(3) <= iris::DataObject(3));
        }

        SECTION("Floats") {
            REQUIRE(iris::DataObject(3.0) >= iris::DataObject(2.0));
            REQUIRE(iris::DataObject(2.0) <= iris::DataObject(3.0));
            REQUIRE_FALSE(iris::DataObject(3.0) <= iris::DataObject(2.0));
            REQUIRE_FALSE(iris::DataObject(2.0) >= iris::DataObject(3.0));

            REQUIRE(iris::DataObject(3.0) >= iris::DataObject(3.0));
            REQUIRE(iris::DataObject(3.0) <= iris::DataObject(3.0));
        }
    }
}

TEST_CASE("DataObject serialization") {
    SECTION("should not add quotes around strings when doing direct string serialization") {
        REQUIRE(
            iris::DataObject("hewwo \" world").serialize()
            ==
            "hewwo \" world"
        );
        INFO("Simulate a nested call");
        REQUIRE(
            iris::DataObject("hewwo \" world").serialize(true)
            ==
            "\"hewwo \\\" world\""
        );
    }
    SECTION("should add quotes around nested strings") {
        iris::DataObject o {
            "hi",
            "\"there"
        };
        REQUIRE(o.serialize() == "[\"hi\",\"\\\"there\"]");
    }
    SECTION("should serialize bools properly") {
        iris::DataObject o(true);
        REQUIRE(o.serialize() == "true");
    }
    SECTION("should serialize ints properly") {
        iris::DataObject signedPositive(1);
        iris::DataObject signedNegative(-1);
        iris::DataObject unsignedNum((uint32_t) 1);

        REQUIRE(signedPositive.serialize() == "1");
        REQUIRE(signedNegative.serialize() == "-1");
        REQUIRE(unsignedNum.serialize() == "1");
    }
    SECTION("should serialize floats properly") {
        iris::DataObject positiveFloat(6.9);
        iris::DataObject negativeFloat(-6.9);
        // to_string adds a bunch of decimals, so can't do an ==
        REQUIRE(positiveFloat.serialize().starts_with("6.9"));
        REQUIRE(negativeFloat.serialize().starts_with("-6.9"));
    }
    SECTION("should serialize pointers to a pointer ref") {
        iris::DataObject obj = iris::Object {
            {"key", iris::DataPointer("a.str")}
        };
        iris::DataObject renderData = iris::Object {
            {"a", iris::Object {
                {"str", "trans rights are human rights"}
            }}
        };

        REQUIRE(obj.serialize() == R"({"key":a.str})");
        REQUIRE(obj.serialize(false, &renderData) == R"({"key":"trans rights are human rights"})");
        
    }

    SECTION("Should serialize null correctly") {
        iris::DataObject obj = iris::Object {
            {"key", nullptr}
        };

        REQUIRE(obj.serialize() == R"({"key":null})");
    }
}

}
