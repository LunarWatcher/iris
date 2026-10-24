add_library(iris
    iris/data/DataObject.cpp
    iris/data/DataPointer.cpp

    iris/config.cpp
    iris/environment.cpp
    iris/function_storage.cpp
    iris/lexer.cpp
    iris/renderer.cpp
    iris/parser.cpp
)
add_library(iris::iris ALIAS iris)

target_include_directories(iris
PUBLIC
    ./
)
