#include "DataPointer.hpp"

#include <sstream>

namespace iris {

bool DataPointer::operator==(const DataPointer& other) const {
    return path == other.path;
}
bool DataPointer::operator<(const DataPointer& other) const {
    return path < other.path;
}
bool DataPointer::operator>(const DataPointer& other) const {
    return path > other.path;
}

bool DataPointer::operator<=(const DataPointer& other) const {
    return path <= other.path;
}
bool DataPointer::operator>=(const DataPointer& other) const {
    return path >= other.path;
}

std::string DataPointer::join() const {
    std::stringstream ss;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i != 0) {
            ss << ".";
        }
        ss << path.at(i);
    }
    return ss.str();
}

}
