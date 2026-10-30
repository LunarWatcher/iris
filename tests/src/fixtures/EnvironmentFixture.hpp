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
            nullptr,
            identifier
        );
    }

    std::string renderWithUdata(
        const std::string& tmpl,
        void* udata,
        const iris::DataObject& obj = iris::Object {},
        const std::string& identifier = ""
    ) {
        return env.render(
            tmpl,
            obj,
            udata,
            identifier
        );
    }

    std::string renderWithUdata(
        iris::Template& tmpl,
        void* udata,
        const iris::DataObject& obj = iris::Object {}
    ) {
        return env.render(
            tmpl,
            obj,
            udata
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
    std::string renderFileWithUData(
        const std::string& filename,
        void* udata,
        const iris::DataObject& obj = iris::Object {}
    ) {
        return env.render_file(resolvePath(filename), obj, udata);
    }

    iris::Template parseString(const std::string& content, const std::string& identifier = "") {
        return env.parse(content, identifier);
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
