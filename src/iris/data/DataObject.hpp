#pragma once

#include "iris/data/DataPointer.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace iris {

class DataObject;

using String = std::string;
using Array = std::vector<DataObject>;
using Object = std::unordered_map<std::string, DataObject>;
using Boolean = bool;
using Int = int64_t;
using Float = double;
using Null = std::nullptr_t;

using StringPtr = std::shared_ptr<String>;
using ArrayPtr = std::shared_ptr<Array>;
using ObjectPtr = std::shared_ptr<Object>;


template <typename T>
concept IsPointerType =
    std::is_same_v<T, StringPtr>
    || std::is_same_v<T, ArrayPtr>
    || std::is_same_v<T, ObjectPtr>;

/**
 * TODOs:
 * * Can we make two iterators instead of one, where one is basically a value iterator and the other is a key-value
 *   iterator? A hybrid iterator appears moot (and an attempt to do so failed and was removed)
 */
class DataObject {
private:
    std::variant<
        StringPtr,
        ArrayPtr,
        ObjectPtr,
        Boolean,
        Int,
        Float,
        Null
    > data;
    void destroy();
public:
    DataObject();

    DataObject(const String& value);
    DataObject(const char* data) : DataObject(std::string { data }) {}
    DataObject(String&& value);

    DataObject(const Array& value);
    DataObject(Array&& value);

    DataObject(const Object& value);
    DataObject(Object&& value);

    DataObject(Null null);

    DataObject(Boolean value);

    template <std::floating_point T>
    DataObject(T value) : data((Float) value) {}

    template <std::integral T>
    DataObject(T value) : data((Int) value) {}

    ~DataObject();

    DataObject& operator=(const DataObject& other);
    DataObject& operator=(DataObject&& other) noexcept;

    bool operator==(const DataObject& other) const;
    bool operator<=(const DataObject& other) const;
    bool operator>=(const DataObject& other) const;
    bool operator<(const DataObject& other) const;
    bool operator>(const DataObject& other) const;
    bool operator!=(const DataObject& other) const;

    DataObject(std::initializer_list<DataObject> list);
    // TODO: because of how the objects are constructed here, there's no way to tell a list from an object, so automatic
    // resolution for this constructor fails
    // DataObject(std::initializer_list<Object::value_type> list) : DataObject(Object(list)) {}

    DataObject(const DataObject& src);
    DataObject(DataObject&& src) noexcept;

    template <typename T>
    const T& get() const {
        if constexpr (
            std::is_same_v<String, T>
            || std::is_same_v<Object, T>
            || std::is_same_v<Array, T>
        ) {
            return *std::get<std::shared_ptr<T>>(data);
        } else {
            return std::get<T>(data);
        }
    }
    template <typename T>
    T& get() {
        if constexpr (
            std::is_same_v<String, T>
            || std::is_same_v<Object, T>
            || std::is_same_v<Array, T>
        ) {
            return *std::get<std::shared_ptr<T>>(data);
        } else {
            return std::get<T>(data);
        }
    }

    bool is_null() const { return std::holds_alternative<Null>(data); }
    bool is_int() const { return std::holds_alternative<Int>(data); }
    bool is_float() const { return std::holds_alternative<Float>(data); }
    bool is_bool() const { return std::holds_alternative<Boolean>(data); }
    bool is_string() const { return std::holds_alternative<StringPtr>(data); }
    bool is_array() const { return std::holds_alternative<ArrayPtr>(data); }
    bool is_object() const { return std::holds_alternative<ObjectPtr>(data); }
    bool is_number() const { return is_int() || is_float(); }

    std::string type() const;

    Int coerceInt() const;
    Float coerceFloat() const;

    bool empty() const;

    const DataObject& at(const DataPointer& ptr, size_t offset = 0) const;

    DataObject& get(const DataPointer& ptr, size_t offset = 0);

    DataObject& at(size_t pos) const;

    DataObject& front() const;
    DataObject& back() const;

    DataObject& at(const std::string& key) const;

    bool contains(const std::string& key) const;
    bool contains(const DataPointer& ptr, size_t offset = 0) const;

    void push_back(const DataObject& obj);

    size_t size() const;

    void clear();

    DataObject& operator[](const DataPointer& ptr);
    DataObject& operator[](const std::string& key);
    DataObject& operator[](size_t pos) const;

    /**
     * Contains JSON serialization.
     */
    std::string serialize(bool nested = false) const;
    // operator std::string() const {
    //     return serialize();
    // }
};

inline std::ostream& operator<<(std::ostream& out, const DataObject& obj) {
    return out << obj.serialize();
}

}
