add_library(iris INTERFACE)
add_library(iris::iris ALIAS iris)

target_include_directories(iris INTERFACE
  ./
)

if(IRIS_USE_EMBEDDED_JSON)
  find_package(nlohmann_json QUIET)
  if(NOT nlohmann_json_FOUND)
    set(IRIS_PACKAGE_USE_EMBEDDED_JSON ON)
    add_library(nlohmann_json INTERFACE)
    add_library(pantor::nlohmann_json ALIAS nlohmann_json)

    target_include_directories(nlohmann_json INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third_party/include>
      $<INSTALL_INTERFACE:${IRIS_INSTALL_INCLUDE_DIR}/nlohmann>
    )

    target_compile_features(nlohmann_json INTERFACE cxx_std_17)

    set(IRIS_SELECTED_JSON_LIBRARY "pantor::nlohmann_json")
  else()
    set(IRIS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")
  endif()
else()
  # If target already exists, e.g. by git submodules
  if(TARGET nlohmann_json)
    set(IRIS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")
  else()
    find_package(nlohmann_json REQUIRED)
    set(IRIS_SELECTED_JSON_LIBRARY "nlohmann_json::nlohmann_json")
  endif()
endif()

target_link_libraries(iris INTERFACE ${IRIS_SELECTED_JSON_LIBRARY})
