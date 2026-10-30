#pragma once

#include "iris/environment.hpp"
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace tests {

struct EnvironmentFixture {
    iris::Environment env;

    EnvironmentFixture() {
        env.set_trim_blocks(true);
    }

    iris::Template parse(
        const std::string& content,
        const std::string& filePathOrIdentifier
    ) {
        return env.parse(
            content,
            filePathOrIdentifier
        );
    }

    std::string render(
        const std::string& tmpl,
        const iris::DataObject& obj = iris::Object {},
        const std::string& identifier = ""
    ) {
        return env.render(
            tmpl,
            obj,
            identifier
        );
    }

    std::filesystem::path resolvePath(const std::string& filename) {
        return std::filesystem::path(TEST_SOURCE) / "data" / filename;
    }

    std::string renderFile(
        const std::string& filename,
        const iris::DataObject& obj = iris::Object {}
    ) {
        return env.render_file(resolvePath(filename), obj);
    }

    iris::Template parseFile(const std::string& filename) {
        auto loc = resolvePath(filename);
        INFO("File location: " << loc.string());
        return env.parse_file(loc);
    }

    std::string loadFile(
        const std::string& filename
    ) {
        auto loc = resolvePath(filename);
        INFO("File location: " << loc.string());
        return env.load_file(loc);
    }
};

}
