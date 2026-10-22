#pragma once

#include "iris/utils.hpp"
#include <string>
#include <vector>
namespace iris {

class DataObject;
/**
 * Meta object used to represent a pointer to a data object.
 * Third party libraries using iris should NEVER declare a DataPointer; this is purely meant for internal
 * use. Initializing anything with a DataPointer is a great way to run into all kinds of problems.
 *
 * The input is a `.`-separated string, and is used to access nested variables.
 */
struct DataPointer {
    std::vector<std::string> path;

    explicit DataPointer(std::string_view src) {
        do {
            std::string_view part;
            // TODO: std::ranges::split? Pretty sure that's C++20
            std::tie(part, src) = string_view::split(
                src,
                '.'
            );
            path.push_back(std::string(part.begin(), part.end()));
        } while (!src.empty());
    }

    // TODO: not sure if these are necessary
    explicit DataPointer(const std::vector<std::string>& path) : path(path) {}
    explicit DataPointer(std::vector<std::string>&& path) : path(std::move(path)) {}

    bool operator==(const DataPointer& other) const;
    bool operator<(const DataPointer& other) const;
    bool operator>(const DataPointer& other) const;
    bool operator<=(const DataPointer& other) const;
    bool operator>=(const DataPointer& other) const;
};


}
