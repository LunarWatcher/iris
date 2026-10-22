#include "DataObject.hpp"
#include <iomanip>
#include <variant>
#include <type_traits>
#include <sstream>
#include <format>

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

DataObject::DataObject(const DataPointer& dataPointer) : data(dataPointer) {}
DataObject::DataObject(DataPointer&& dataPointer) : data(std::move(dataPointer)) {}

DataObject::DataObject(Boolean value) : data(value) {}

DataObject::DataObject(std::initializer_list<DataObject> list) : DataObject(Array(list)) {}

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
                acc << std::quoted(k, '"', '\\') << ":" << v.serialize(true, renderData);
            }
            acc << '}';
        } else if constexpr (std::is_same_v<ArrayPtr, T>) {
            acc << '[';
            for (size_t i = 0; i < value->size(); ++i) {
                if (i != 0) {
                    acc << ',';
                }
                acc << value->at(i).serialize(true, renderData);
            }
            acc << ']';
        } else if constexpr (std::is_same_v<DataPointer, T>) {
            if (renderData != nullptr) {
                acc << (*renderData)[value].serialize(nested, renderData);
            } else {
                for (size_t i = 0; i < value.path.size(); ++i) {
                    if (i != 0) {
                        acc << ".";
                    }
                    acc << value.path.at(i);
                }
            }
        } else if constexpr (std::is_same_v<Boolean, T>) {
            acc << (value ? "true" : "false");
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
    }, this->data, other.data);
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
    }, this->data, other.data);
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
    }, this->data, other.data);
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
    }, this->data, other.data);
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
    }, this->data, other.data);
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
    }, this->data, other.data);
}


Int DataObject::coerceInt() const {
    if (std::holds_alternative<Int>(data)) {
        return std::get<Int>(data);
    } else if (std::holds_alternative<Float>(data)) {
        return (Int) std::get<Float>(data);
    } else if (std::holds_alternative<UInt>(data)) {
        return (Int) std::get<UInt>(data);
    } else {
        throw std::runtime_error("Not a numeric type");
    }
}

Float DataObject::coerceFloat() const {
    if (std::holds_alternative<Int>(data)) {
        return (Float) std::get<Int>(data);
    } else if (std::holds_alternative<Float>(data)) {
        return std::get<Float>(data);
    } else if (std::holds_alternative<UInt>(data)) {
        return (Float) std::get<UInt>(data);
    } else {
        throw std::runtime_error("Not a numeric type");
    }
}


DataObject& DataObject::operator[](const std::string& key) {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            if (!value->contains(key)) {
                (*value)[key] = DataObject();
            }
            return value->at(key);
        } else {
            throw std::runtime_error("Critical: called operator[str] on a non-object");
        }
    }, data);
}

DataObject& DataObject::operator[](size_t pos) const {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            return value->at(pos);
        } else {
            throw std::runtime_error("Critical: called operator[int] on a non-object");
        }
    }, data);
}
const DataObject& DataObject::at(const DataPointer& ptr, size_t offset) const {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            auto& it = at(ptr.path.at(offset));

            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            // TODO: error handle
            size_t index = std::stoull(ptr.path.at(offset));

            auto& it = at(index);
            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else {
            throw std::runtime_error("Critical: called operator[str] on a non-object");
        }
    }, data);
}

DataObject& DataObject::get(const DataPointer& ptr, size_t offset) {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            auto& it = at(ptr.path.at(offset));

            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            // TODO: error handle
            size_t index = std::stoull(ptr.path.at(offset));

            auto& it = at(index);
            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else {
            throw std::runtime_error("Critical: called operator[str] on a non-object");
        }
    }, data);
}

DataObject& DataObject::operator[](const DataPointer& ptr) {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
            || std::is_same_v<T, ArrayPtr>
        ) {
            return get(ptr, 0);
        } else {
            throw std::runtime_error("Critical: called operator[str] on a non-object");
        }
    }, data);
}

DataObject& DataObject::at(size_t pos) const {
    return (*this)[pos];
}

DataObject& DataObject::front() const {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            return value->front();
        } else {
            throw std::runtime_error("Critical: called front on a non-object");
        }
    }, data);
}
DataObject& DataObject::back() const {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            return value->back();
        } else {
            throw std::runtime_error("Critical: called back on a non-object");
        }
    }, data);
}

DataObject& DataObject::at(const std::string& key) const {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            if (!value->contains(key)) {
                throw std::runtime_error(
                    std::format("Illegal access on key {}", key)
                );
            }
            return value->at(key);
        } else {
            throw std::runtime_error("Critical: called at[str] on a non-object");
        }
    }, data);
}

bool DataObject::contains(const std::string& key) const {
    return std::visit([&](const auto& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            return value->contains(key);

        } else {
            throw std::runtime_error("Critical: called exists on a non-object");
        }
    }, data);
}

bool DataObject::contains(const DataPointer& ptr, size_t offset) const {
    return std::visit([&](const auto& value) -> bool {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {
            if (!contains(ptr.path.at(offset))) {
                return false;
            }
            if (offset == ptr.path.size() - 1) {
                return true;
            }
            const auto& it = at(ptr.path.at(offset));
            return it.contains(ptr, offset + 1);
        } else if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            // TODO: error handle
            size_t index = std::stoull(ptr.path.at(offset));

            if (value->size() <= index) {
                return false;
            }
            if (offset == ptr.path.size() - 1) {
                return true;
            }
            const auto& it = at(index);
            return it.contains(ptr, offset + 1);
        } else {
            throw std::runtime_error("Critical: called operator[str] on a non-object");
        }
    }, data);
}

size_t DataObject::size() const {
    return std::visit([&](const auto& value) -> size_t {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
            || std::is_same_v<T, ArrayPtr>
            || std::is_same_v<T, StringPtr>
        ) {
            return value->size();
        } else {
            throw std::runtime_error("Critical: called exists on a non-string/array/object");
        }
    }, data);
}

void DataObject::clear() const {
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
            || std::is_same_v<T, ArrayPtr>
            || std::is_same_v<T, StringPtr>
        ) {
            value->clear();
        } else {
            throw std::runtime_error("Critical: called exists on a non-string/array/object");
        }
    }, data);
}

}
