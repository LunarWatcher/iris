add_executable(tests
    src/Main.cpp

    src/data/DataObjectTests.cpp

    src/rendering/FileLoadingTests.cpp
    src/rendering/ForLoopTests.cpp
    src/rendering/LogicTests.cpp
    src/rendering/OperatorTests.cpp
    src/rendering/SetTests.cpp
    src/rendering/SubstitutionTests.cpp

    src/rendering/errors/ErrorMessageTests.cpp
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.16.1
)

FetchContent_MakeAvailable(Catch2)

target_link_libraries(tests Catch2::Catch2 iris::iris)

target_include_directories(tests PUBLIC src/)

target_compile_definitions(tests PUBLIC -DTEST_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}")

# vim:ft=cmake
