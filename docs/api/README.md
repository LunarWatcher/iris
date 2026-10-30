---
{
    "title": "API documentation"
}
---
# API documentation

This folder contains API docs for iris. This is meant for developers trying to embed iris, NOT developers using the iris language. For the language, see [the language documentation](/language) instead.

## Integration

The suggested way to use iris is with FetchContent:
```cmake
include(FetchContent)
FetchContent_Declare(iris
    GIT_REPOSITORY https://codeberg.org/LunarWatcher/iris
    GIT_TAG v0.2.0
)
FetchContent_MakeAvailable(iris)
# ...
target_include_libraries(your-target PUBLIC iris::iris)
```

### Requirements

The only requirement is a C++23 compiler. Iris should work under windows, but this has not been tested since the fork. At this time, iris is dependency-free. A logging library may be added in the future, but this will be via FetchContent, so no further changes to the CMake setup should be necessary.

