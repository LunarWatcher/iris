#pragma once

#include "iris/environment.hpp"

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <sstream>

namespace tests {

struct FileFixture {
    iris::Environment env;

    std::filesystem::path resolvePath(const std::string& filename) {
        return std::filesystem::path(TEST_SOURCE) / "data" / filename;
    }

    iris::Template parseFile(const std::string& filename) {
        auto loc = resolvePath(filename);
        INFO("File location: " << loc.string());
        return env.parse_file(loc);
    }

    std::string loadFile(const std::string& filename) {
        auto loc = resolvePath(filename);
        INFO("File location: " << loc.string());
        return env.load_file(loc);
    }
};

}
