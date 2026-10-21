#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

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
using StringPtr = std::shared_ptr<std::string>;
using ArrayPtr = std::shared_ptr<std::vector<DataObject>>;
using ObjectPtr = std::shared_ptr<std::unordered_map<std::string, DataObject>>;

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
        NullPtr
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

    bool is_null() const { return std::holds_alternative<NullPtr>(data); }
    bool is_int() const { return std::holds_alternative<Int>(data); }
    bool is_uint() const { return std::holds_alternative<UInt>(data); }
    bool is_float() const { return std::holds_alternative<Float>(data); }
    bool is_bool() const { return std::holds_alternative<Boolean>(data); }
    bool is_string() const { return std::holds_alternative<StringPtr>(data); }
    bool is_array() const { return std::holds_alternative<ArrayPtr>(data); }
    bool is_object() const { return std::holds_alternative<ObjectPtr>(data); }

    /**
     * Contains JSON serialization.
     */
    std::string serialize(bool nested = false) const;
    operator std::string() const {
        return serialize();
    }
};

inline std::ostream& operator<<(std::ostream& out, const DataObject& obj) {
    return out << obj.serialize();
}

}
