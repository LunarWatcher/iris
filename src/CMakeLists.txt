add_library(iris
    ./iris/data/DataObject.cpp
    ./iris/data/DataPointer.cpp
)
add_library(iris::iris ALIAS iris)

target_include_directories(iris
PUBLIC
    ./
)
