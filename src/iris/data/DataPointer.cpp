#include "DataPointer.hpp"

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

}
