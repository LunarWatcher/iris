cmake_minimum_required(VERSION 3.13)

project(iris
        DESCRIPTION "Template engine for modern C++"
        LANGUAGES CXX
        VERSION 1.0.0)


option(COVERALLS "Generate coveralls data" OFF)
option(IRIS_BUILD_TESTS "Whether or not to buidl tests" OFF)
option(IRIS_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(IRIS_INSTALL "Generate install targets for iris" ON)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "iris: Running standalone")
    set(IRIS_STANDALONE ON)

    if (NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fsanitize=undefined")
    endif()

    option(MAGPIE_BUILD_TESTS "Whether or not to build tests" ON)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
else()
    message (STATUS "iris: Running as a submodule")
    set(IRIS_STANDALONE OFF)

    option(IRIS_BUILD_TESTS "Whether or not to build tests" OFF)
endif()


set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# For using the correct __cplusplus macro
# TODO: is this even necessary?
if(MSVC)
  add_compile_options(/utf-8 /Zc:__cplusplus)
endif()

add_subdirectory(src)

if(IRIS_BUILD_TESTS)
    add_subdirectory(tests)
    add_custom_target(test
        COMMAND tests
        DEPENDS tests
        COMMENT "Test iris")
endif()

