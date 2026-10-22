#include "DataObject.hpp"
#include <iomanip>
#include <variant>
#include <type_traits>
#include <sstream>

namespace iris {

DataObject::~DataObject() {
    // TODO: this was logical in earlier versions of this struct, but I don't think it's necessary anymore
    if (std::holds_alternative<NullPtr>(data)) {
        return;
    }
    this->data = DataObject::null_t;
}

DataObject::DataObject(const String& value) : data(std::make_shared<String>(value)) {}
DataObject::DataObject(String&& value) : data(std::make_shared<String>(std::move(value))) {}

DataObject::DataObject(const Array& value) : data(std::make_shared<Array>(value)) {}
DataObject::DataObject(Array&& value) : data(std::make_shared<Array>(std::move(value))) {}

DataObject::DataObject(const Object& value) : data(std::make_shared<Object>(value)) {}
DataObject::DataObject(Object&& value) : data(std::make_shared<Object>(std::move(value))) {}

DataObject::DataObject(const NullPtr& null) : data(null) {}
DataObject::DataObject() : data(DataObject::null_t) {}

DataObject& DataObject::operator=(const DataObject& other) {
    std::visit([&](auto& value) {
        using type = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<type, Null*>
            || !std::is_pointer_v<type>
        ) {
            data = value;
        } else if constexpr (std::is_same_v<type, StringPtr>) {
            data = std::make_shared<String>(value->begin(), value->end());
        } else if constexpr (std::is_same_v<type, ArrayPtr>) {
            auto vec = std::make_shared<Array>();
            *vec = *value;
            data = vec;
        } else if constexpr (std::is_same_v<type, ObjectPtr>) {
            auto map = std::make_shared<Object>();
            *map = *value;
            data = map;
        } else {
            // Unhandled type
            static_assert(false);
        }
    }, other.data);
    return *this;
}
DataObject& DataObject::operator=(DataObject&& other) noexcept {
    this->data = other.data;
    other.data = DataObject::null_t;
    return *this;
}

DataObject::DataObject(const DataObject& src) : data(src.data) {}
DataObject::DataObject(DataObject&& src) noexcept : data(std::move(src.data)) {
    src.data = null_t;
}

std::string DataObject::serialize(bool nested, DataObject* renderData) const {
    std::stringstream acc;

    std::visit([&](const auto& value) -> void {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<NullPtr, T>) {
            acc << "null";
        } else if constexpr (std::is_same_v<StringPtr, T>) {
            if (!nested) {
                acc << *value;
            } else {
                acc << std::quoted(*value, '"', '\\');
            }
        } else if constexpr (std::is_same_v<ObjectPtr, T>) {
            bool hasFirst = false;
            acc << '{';
            for (const auto& [k, v] : *value) {
                if (hasFirst) {
                    acc << ',';
                }
                hasFirst = true;
                acc << std::quoted(k, '"', '\\') << ":" << v.serialize(nested = true);
            }
            acc << '}';
        } else if constexpr (std::is_same_v<ArrayPtr, T>) {
            acc << '[';
            for (size_t i = 0; i < value->size(); ++i) {
                if (i != 0) {
                    acc << ',';
                }
                acc << value->at(i).serialize(nested = true);
            }
            acc << ']';
        } else if constexpr (std::is_same_v<DataPointer, T>) {
            if (renderData != nullptr) {
                acc << (*renderData)[value].serialize();
            } else {
                for (size_t i = 0; i < value.path.size(); ++i) {
                    if (i != 0) {
                        acc << ".";
                    }
                    acc << value.path.at(i);
                }
            }
        } else {
            acc << std::to_string(value);
        }
    }, data);
    
    return acc.str();
}

bool DataObject::empty() const {
    return std::visit([](const auto& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, StringPtr>
            || std::is_same_v<T, ObjectPtr>
            || std::is_same_v<T, ArrayPtr>
        ) {
            return value->empty();
        } else {
            throw std::runtime_error("Critical: called empty on a non-string/array type");
        }
    }, data);
}

bool DataObject::operator==(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, ArrayPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                return (*a) == (*b);
            } else {
                return a == b;
            }
        } else {
            return false;
        }
    }, other.data, this->data);
}

bool DataObject::operator<=(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ArrayPtr>
            ) {
                return (*a) <= (*b);
            } else if constexpr (
                std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                throw std::runtime_error("operator<= on objects and null is undefined");
            } else {
                return a <= b;
            }
        } else {
            return false;
        }
    }, other.data, this->data);
}
bool DataObject::operator>=(const DataObject& other) const {
    
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ArrayPtr>
            ) {
                return (*a) >= (*b);
            } else if constexpr (
                std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                throw std::runtime_error("operator>= on objects and null is undefined");
            } else {
                return a >= b;
            }
        } else {
            return false;
        }
    }, other.data, this->data);
}
bool DataObject::operator<(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ArrayPtr>
            ) {
                return (*a) > (*b);
            } else if constexpr (
                std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                throw std::runtime_error("operator< on objects and null is undefined");
            } else {
                return a < b;
            }
        } else {
            return false;
        }
    }, other.data, this->data);
}
bool DataObject::operator>(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ArrayPtr>
            ) {
                return (*a) > (*b);
            } else if constexpr (
                std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                throw std::runtime_error("operator> on objects and null is undefined");
            } else {
                return a > b;
            }
        } else {
            return false;
        }
    }, other.data, this->data);
}
bool DataObject::operator!=(const DataObject& other) const {
    
    return std::visit([](const auto& a, const auto& b) {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (std::is_same_v<t1, t2>) {
            if constexpr (
                std::is_same_v<t1, StringPtr>
                || std::is_same_v<t1, ObjectPtr>
                || std::is_same_v<t1, ArrayPtr>
                || std::is_same_v<t1, NullPtr>
            ) {
                return (*a) != (*b);
            } else {
                return a != b;
            }
        } else {
            return true;
        }
    }, other.data, this->data);
}

}
