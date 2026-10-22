#pragma once

#include "iris/data/DataPointer.hpp"
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <iterator>

namespace iris {

struct Null {
    Null() noexcept = default;

    bool operator==(const Null&) const { return true; }
};

class DataObject;

using String = std::string;
using Array = std::vector<DataObject>;
using Object = std::unordered_map<std::string, DataObject>;
using Boolean = bool;
using Int = int64_t;
using UInt = uint64_t;
using Float = double;

using NullPtr = std::shared_ptr<Null>;

using StringPtr = std::shared_ptr<String>;
using ArrayPtr = std::shared_ptr<Array>;
using ObjectPtr = std::shared_ptr<Object>;



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
        UInt,
        Float,
        NullPtr,
        DataPointer
    > data;
    void destroy();
public:

    static inline NullPtr null_t = std::make_shared<Null>();

    DataObject();

    DataObject(const String& value);
    DataObject(const char* data) : DataObject(std::string { data }) {}
    DataObject(String&& value);

    DataObject(const Array& value);
    DataObject(Array&& value);

    DataObject(const Object& value);
    DataObject(Object&& value);

    DataObject(const NullPtr& null);
    // TODO: this can be private with some `friend class`es since it's meant to be internal use only
    DataObject(const DataPointer& dataPointer);

    DataObject(Boolean value) : data(value) {}

    template <std::floating_point T>
    DataObject(T value) : data((Float) value) {}

    template <std::signed_integral T>
    DataObject(T value) : data((Int) value) {}

    template <std::unsigned_integral T>
    DataObject(T value) : data((UInt) value) {}

    ~DataObject();

    DataObject& operator=(const DataObject& other);
    DataObject& operator=(DataObject&& other) noexcept;

    bool operator==(const DataObject& other) const;
    bool operator<=(const DataObject& other) const;
    bool operator>=(const DataObject& other) const;
    bool operator<(const DataObject& other) const;
    bool operator>(const DataObject& other) const;
    bool operator!=(const DataObject& other) const;

    DataObject(std::initializer_list<DataObject> list) : DataObject(Array(list)) {}
    // TODO: because of how the objects are constructed here, there's no way to tell a list from an object, so automatic
    // resolution for this constructor fails
    // DataObject(std::initializer_list<Object::value_type> list) : DataObject(Object(list)) {}

    explicit DataObject(const DataObject& src);
    explicit DataObject(DataObject&& src) noexcept;

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

    bool is_null() const { return std::holds_alternative<NullPtr>(data); }
    bool is_int() const { return std::holds_alternative<Int>(data); }
    bool is_uint() const { return std::holds_alternative<UInt>(data); }
    bool is_float() const { return std::holds_alternative<Float>(data); }
    bool is_bool() const { return std::holds_alternative<Boolean>(data); }
    bool is_string() const { return std::holds_alternative<StringPtr>(data); }
    bool is_array() const { return std::holds_alternative<ArrayPtr>(data); }
    bool is_object() const { return std::holds_alternative<ObjectPtr>(data); }
    bool is_number() const { return is_int() || is_uint() || is_float(); }

    bool is_any_int() const { return is_int() || is_uint(); }

    Int coerceInt() const {
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
    Float coerceFloat() const {
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

    bool empty() const;

    DataObject& operator[](const std::string& key) {
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

    DataObject& operator[](size_t pos) const {
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
    const DataObject& at(const DataPointer& ptr, size_t offset = 0) const {
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

    DataObject& get(const DataPointer& ptr, size_t offset = 0) {
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

    DataObject& operator[](const DataPointer& ptr) {
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

    DataObject& at(size_t pos) const {
        return (*this)[pos];
    }

    DataObject& front() const {
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
    DataObject& back() const {
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

    DataObject& at(const std::string& key) const {
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

    bool contains(const std::string& key) const {
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

    bool contains(const DataPointer& ptr, size_t offset = 0) const {
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

    size_t size() const {
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

    void clear() const {
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

    /**
     * Contains JSON serialization.
     */
    std::string serialize(bool nested = false, DataObject* renderData = nullptr) const;
    operator std::string() const {
        return serialize();
    }
};

inline std::ostream& operator<<(std::ostream& out, const DataObject& obj) {
    return out << obj.serialize();
}

}
