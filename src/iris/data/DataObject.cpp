#include "DataObject.hpp"
#include "iris/data/DataPointer.hpp"
#include "iris/exceptions.hpp"
#include <iomanip>
#include <stdexcept>
#include <variant>
#include <type_traits>
#include <sstream>
#include <format>

namespace iris {

DataObject::~DataObject() {
    this->data = nullptr;
}

DataObject::DataObject(const String& value) : data(std::make_shared<String>(value)) {}
DataObject::DataObject(String&& value) : data(std::make_shared<String>(std::move(value))) {}

DataObject::DataObject(const Array& value) : data(std::make_shared<Array>(value)) {}
DataObject::DataObject(Array&& value) : data(std::make_shared<Array>(std::move(value))) {}

DataObject::DataObject(const Object& value) : data(std::make_shared<Object>(value)) {}
DataObject::DataObject(Object&& value) : data(std::make_shared<Object>(std::move(value))) {}

DataObject::DataObject(Null null) : data(null) {}
DataObject::DataObject() : data(nullptr) {}

DataObject::DataObject(Boolean value) : data(value) {}

DataObject::DataObject(std::initializer_list<DataObject> list) : DataObject(Array(list)) {}

DataObject& DataObject::operator=(const DataObject& other) {
    std::visit([&](auto& value) {
        using type = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<type, Null>
            || !std::is_pointer_v<type>
        ) {
            data = value;
        } else if constexpr (std::is_same_v<type, StringPtr>) {
            data = std::make_shared<String>(value->begin(), value->end());
        } else if constexpr (std::is_same_v<type, ArrayPtr>) {
            // TODO: This implementation is incorrect
            auto vec = std::make_shared<Array>();
            *vec = *value;
            data = vec;
        } else if constexpr (std::is_same_v<type, ObjectPtr>) {
            // TODO: This implementation is incorrect
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
    other.data = nullptr;
    return *this;
}

DataObject::DataObject(const DataObject& src) : data(src.data) {}
DataObject::DataObject(DataObject&& src) noexcept : data(std::move(src.data)) {
    src.data = nullptr;
}

std::string DataObject::serialize(bool nested) const {
    std::stringstream acc;

    std::visit([&](const auto& value) -> void {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Null, T>) {
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
                acc << std::quoted(k, '"', '\\') << ":" << v.serialize(true);
            }
            acc << '}';
        } else if constexpr (std::is_same_v<ArrayPtr, T>) {
            acc << '[';
            for (size_t i = 0; i < value->size(); ++i) {
                if (i != 0) {
                    acc << ',';
                }
                acc << value->at(i).serialize(true);
            }
            acc << ']';
        } else {
            acc << std::format("{}", value);
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
            throw ContextlessRenderError("empty", "called empty on a non-string/array type");
        }
    }, data);
}

bool DataObject::operator==(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (std::is_same_v<t1, t2>) {
                return (*a) == (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return false;
            }
        } else {
            if constexpr (std::equality_comparable_with<t1, t2>) {
                return a == b;
            }
            return false;
        }
    }, this->data, other.data);
}

bool DataObject::operator<=(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (
                std::is_same_v<t1, t2>
                && !std::is_same_v<t1, ObjectPtr>
            ) {
                return (*a) <= (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return false;
            }
        } else {
            if constexpr (std::totally_ordered_with<t1, t2>) {
                return a <= b;
            }
            return false;
        }
    }, this->data, other.data);
}
bool DataObject::operator>=(const DataObject& other) const {
    
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (
                std::is_same_v<t1, t2>
                && !std::is_same_v<t1, ObjectPtr>
            ) {
                return (*a) >= (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return false;
            }
        } else {
            if constexpr (std::totally_ordered_with<t1, t2>) {
                return a >= b;
            }
            return false;
        }
    }, this->data, other.data);
}
bool DataObject::operator<(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (
                std::is_same_v<t1, t2>
                && !std::is_same_v<t1, ObjectPtr>
            ) {
                return (*a) < (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return false;
            }
        } else {
            if constexpr (std::totally_ordered_with<t1, t2>) {
                return a < b;
            }
            return false;
        }
    }, this->data, other.data);
}
bool DataObject::operator>(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) -> bool {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (
                std::is_same_v<t1, t2>
                && !std::is_same_v<t1, ObjectPtr>
            ) {
                return (*a) > (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return false;
            }
        } else {
            if constexpr (std::totally_ordered_with<t1, t2>) {
                return a > b;
            }
            return false;
        }
    }, this->data, other.data);
}
bool DataObject::operator!=(const DataObject& other) const {
    return std::visit([](const auto& a, const auto& b) {
        using t1 = std::decay_t<decltype(a)>;
        using t2 = std::decay_t<decltype(b)>;
        if constexpr (IsPointerType<t1> || IsPointerType<t2>) {
            if constexpr (std::is_same_v<t1, t2>) {
                return (*a) != (*b);
            } else {
                // The pointer types are all complex enough that there are no trivial conversions, so we don't need to
                // do anything else here
                return true;
            }
        } else {
            if constexpr (std::equality_comparable_with<t1, t2>) {
                return a != b;
            }
            return true;
        }
    }, this->data, other.data);
}


Int DataObject::coerceInt() const {
    if (std::holds_alternative<Int>(data)) {
        return std::get<Int>(data);
    } else if (std::holds_alternative<Float>(data)) {
        return (Int) std::get<Float>(data);
    } else {
        throw ContextlessRenderError("convert", "Not a numeric type");
    }
}

Float DataObject::coerceFloat() const {
    if (std::holds_alternative<Int>(data)) {
        return (Float) std::get<Int>(data);
    } else if (std::holds_alternative<Float>(data)) {
        return std::get<Float>(data);
    } else {
        throw ContextlessRenderError("convert", "Not a numeric type");
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
            throw ContextlessRenderError(
                "access",
                std::format("called operator[str] on a non-object w/key {}", key)
            );
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
            throw ContextlessRenderError("access", "called operator[int] on a non-array");
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
            auto sizeResult = parseSize(ptr.path.at(offset), ptr);
            if (!sizeResult.has_value()) {
                throw ContextlessRenderError(
                    "access",
                    sizeResult.error()
                );
            }

            auto& it = at(sizeResult.value());
            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else {
            throw ContextlessRenderError(
                "access",
                std::format("called at[str] on a non-object with access key {}", ptr.join())
            );
        }
    }, data);
}

DataObject& DataObject::get(const DataPointer& ptr, size_t offset) {
    return std::visit([&](const auto& value) -> DataObject& {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
        ) {

            if (offset == ptr.path.size() - 1) {
                auto& it = (*this)[ptr.path.at(offset)];
                return it;
            }
            auto& it = at(ptr.path.at(offset));
            return it.get(ptr, offset + 1);
        } else if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            auto sizeResult = parseSize(ptr.path.at(offset), ptr);

            if (!sizeResult.has_value()) {
                throw ContextlessRenderError(
                    "access",
                    sizeResult.error()
                );
            }

            auto& it = at(sizeResult.value());
            if (offset == ptr.path.size() - 1) {
                return it;
            }
            return it.get(ptr, offset + 1);
        } else {
            throw ContextlessRenderError(
                "access",
                std::format("called get[str] on a non-object with access key {}", ptr.join())
            );
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
            throw ContextlessRenderError(
                "access",
                std::format("called operator[ptr] on a non-object with key {}", ptr.join())
            );
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
            throw ContextlessRenderError("access", "called front on a non-object");
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
            throw ContextlessRenderError("access", "called back on a non-object");
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
                throw ContextlessRenderError(
                    "access",
                    std::format("Illegal access on key {}", key)
                );
            }
            return value->at(key);
        } else {
            throw ContextlessRenderError("access", "called at[str] on a non-object");
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
            throw ContextlessRenderError("contains", "called exists on a non-object");
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
            auto indexResult = parseSize(ptr.path.at(offset), ptr);

            if (!indexResult.has_value()) {
                return false;
            }

            auto index = indexResult.value();

            if (value->size() <= index) {
                return false;
            }
            if (offset == ptr.path.size() - 1) {
                return true;
            }
            const auto& it = at(index);
            return it.contains(ptr, offset + 1);
        } else {
            throw ContextlessRenderError("acecss", "called contains[ptr] on a non-object");
        }
    }, data);
}

std::expected<size_t, std::string> DataObject::parseSize(
    const std::string& src,
    const DataPointer& ptr
) {
    size_t offset = 0;
    try {
        auto result = std::stoull(src, &offset);
        if (offset != src.size()) {
            return std::unexpected(
                std::format(
                    "expected valid number, but found \"{}\" while accessing path \"{}\"",
                    src,
                    ptr.join()
                )
            );
        }
        return result;
    } catch (const std::invalid_argument& e) {
        return std::unexpected(
            std::format(
                "expected valid number, but found \"{}\" while accessing path \"{}\"",
                src,
                ptr.join()
            )
        );
    } catch (const std::out_of_range& e) {
        return std::unexpected(
            std::format(
                "array index out of range in path {}",
                ptr.join()
            )
        );
    }
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
            throw ContextlessRenderError(
                "access",
                std::format(
                    "called exists on a non-string/array/object (actual type: {})",
                    type()
                )
            );
        }
    }, data);
}

void DataObject::clear() {
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ObjectPtr>
            || std::is_same_v<T, ArrayPtr>
            || std::is_same_v<T, StringPtr>
        ) {
            value->clear();
        } else {
            (*this) = DataObject(nullptr);
        }
    }, data);
}

void DataObject::push_back(const DataObject& obj) {
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (
            std::is_same_v<T, ArrayPtr>
        ) {
            value->push_back(obj);
        } else {
            throw ContextlessRenderError("push", "Cannot push to non-array");
        }
    }, data);
}

std::string DataObject::type() const {
    return std::visit([&](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, ArrayPtr>) {
            return "Array";
        } else if constexpr (std::is_same_v<T, ObjectPtr>) {
            return "Object";
        } else if constexpr (std::is_same_v<T, StringPtr>) {
            return "String";
        } else if constexpr (std::is_same_v<T, Null>) {
            return "null";
        } else if constexpr (std::is_same_v<T, Int>) {
            return "Int";
        } else if constexpr (std::is_same_v<T, Float>) {
            return "Float";
        } else if constexpr (std::is_same_v<T, Boolean>) {
            return "Boolean";
        }
    }, data);
}

}
