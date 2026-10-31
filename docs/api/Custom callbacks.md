---
{
    "title": "Custom callbacks"
}
---
# Custom callbacks

You can create your own and more complex functions with callbacks. These are implemented with `std::function`, so you can for example use C++ lambdas. Iris `Arguments` are a vector of DataObject pointers.

For more information about `iris::DataObject`, see [its documentation](DataObject.md).

## Basics

```cpp
Environment env;

/*
 * Callbacks are defined by its:
 * - name,
 * - (optional) number of arguments,
 * - callback function.
 */
env.add_callback("double", 1, [](Arguments& args) {
    int number = args.at(0)->get<iris::Int>(); // Adapt the index and type of the argument
    return 2 * number;
});

// You can then use a callback like a regular function
env.render("{{ double(16) }}", data); // "32"

// Iris falls back to variadic callbacks if the number of expected arguments is omitted.
env.add_callback("argmax", [](Arguments& args) {
  auto result = std::max_element(args.begin(), args.end(), [](const iris::DataObject* a, const iris::DataObject* b) { return *a < *b;});
  return std::distance(args.begin(), result);
});
env.render("{{ argmax(4, 2, 6) }}", data); // "2"
env.render("{{ argmax(0, 2, 6, 8, 3) }}", data); // "3"

// A callback without argument can be used like a dynamic variable:
std::string greet = "Hello";
env.add_callback("double-greetings", 0, [greet](Arguments args) {
    return greet + " " + greet + "!";
});
env.render("{{ double-greetings }}", data); // "Hello Hello!"
```

You can also add a void callback without return variable, e.g. for debugging:
```cpp
env.add_void_callback("log", 1, [greet](Arguments args) {
    std::cout << "logging: " << args[0] << std::endl;
});
env.render("{{ log(neighbour) }}", data); // Prints nothing to result, only to cout...
```

## Exceptions

You may want to throw exceptions under certain circumstances that lead to decent error messages that can be read. To do so, throw an `iris::CallbackError`. `CallbackError` is an exception type that's consumed internally by iris to provide a better `RenderError`, with all the error context required to debug the error.

Iris does not provide error rendering; this is left as an exercise to the end-application. However, the `iris::RenderError` contains the information you need to figure out which file to get information from, as well as where in the file the error occurred.

> [!warning]
>
> Do NOT `catch(iris::CallbackError)`; instead, `catch(iris::RenderError)`. The type is converted after being thrown to provide the appropriate render context.

If you throw any other type of exception in a callback, it will not be caught by iris. This can be useful under certain circumstances, but do be aware that this prevents iris from providing any debug information to your exception.

## User data

In some cases, you may want to pass userdata with your render to provide context for functions that would otherwise need to be manually and sloppily managed via global objects with hidden keys. This does technically work, but it's awkward and requires passing redundant arguments that may be modified by the template in unexpected ways, and cause unexpected internal changes.

Rather than doing that, you can use userdata. Userdata is an extra pointer passed to all of the render calls, to whatever struct you want.

```cpp
struct MyDataObject;

env.add_callback("my_callback", 0, [](iris::Arguments& args, void* udata) {
    auto* data = (MyDataObject*) udata;
    // Do whatever with the udata, or return parts of it
    return data->field;
});
MyDataObject data;
env.render(templateContent, iris::Object{}, &data);
```

Please note that any data object you pass to `render` must have a lifetime guaranteed to last until `render` returns.

> [!note]
>
> Because *very* raw pointers are used, there's no type safety. This is largely to allow more flexible use of the userdata pointers across different renders with the same set of functions without being awfully restrictive, but means that if you need to operate on multiple data types, the usual pointer footguns apply.

