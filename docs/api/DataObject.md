---
{
    "title": "DataObject"
}
---
# DataObject

DataObject is the main object used for passing data around iris. It's a generic tree-like container that can store arbitrary hierarchies of data in stdlib-standard types.

## Custom data objects

You cannot directly store custom types in a DataObject at this time. If you want to put your own struct into a DataObject, you'll need to convert it. There's nothing special to doing so, it's just a bit manual.

```cpp
struct Whatever {
    int x;
    std::string y;

    // You don't have to do `operator DataObject`, but I like this because it allows fairly direct use of
    // the struct with other iris data objects
    operator iris::DataObject() {
        return iris::Object { { "x", x }, {"y", y } };
    }
};
```

In future, this may be supported in some way, I just don't know how yet.

## `DataObject` types

Iris uses 7 types internally:

* `iris::String` (alias: std::string)
* `iris::Array`
* `iris::Object`
* `iris::Boolean`
* `iris::Int` (real type: int64_t)
* `iris::Float` (real type: double)

There's also `iris::Null`, but you do not need to worry about this in practice.

If you fail to use these types, you may end up getting incomprehensible error messages.

## Initialization

With the exception of objects, initialization is done directly with the `DataObject` constructor:
```cpp
DataObject(1); // int
DataObject(1.1); // double
DataObject("50fuckingboiledpotatoes"); // string
// Warning: initializer lists currently always initialize as an array.
DataObject{ "it1", "it2", "it3", DataObject("as any other type") }; // array
DataObject(true); // boolean
DataObject(nullptr); // null (obviously)

// Objects; the initializer lists between the vector and object were too similar for the
// compiler to correctly disambiguate, and couldn't figure out how to support both the same way.
// I also had problems with nlohmann/json (which uses a similar style of object) incorrectly assuming an
// object is an array or vice versa.
DataObject(iris::Object {
    {"key", "value"}
    {"key2", iris::Object {

    }},
    {"key3", 1.1},
    {"key4", nullptr},
});
```




