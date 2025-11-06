cmake_minimum_required(VERSION 3.21)
project(DynamicEncrypt LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(dynamicencrypt_core STATIC
    src/core/VaultManager.cpp
)

target_include_directories(dynamicencrypt_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(dynamicencrypt_core
    PUBLIC
        Qt6::Core
)


add_library(aes_plugin SHARED
    src/plugins/aes_plugin/AESDriverImpl.cpp
)

target_include_directories(aes_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(aes_plugin
    PRIVATE
        Qt6::Core
        dynamicencrypt_core
)

set_target_properties(aes_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins
)

target_sources(aes_plugin
    PRIVATE
        src/plugins/aes_plugin/AESDriverImpl.h
        src/plugins/aes_plugin/plugin.json
)

add_executable(dynamicencrypt
    src/main.cpp
    src/gui/MainWindow.cpp
    src/gui/KeyDialog.cpp
)

target_include_directories(dynamicencrypt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(dynamicencrypt
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        dynamicencrypt_core
)

set_target_properties(dynamicencrypt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(core_tests
    tests/core_tests.cpp
)

target_include_directories(core_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(core_tests
    PRIVATE
        Catch2::Catch2
        Qt6::Core
        dynamicencrypt_core
)

set_target_properties(core_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_dependencies(core_tests aes_plugin)

include(CTest)
if (BUILD_TESTING)
    add_test(NAME core_tests COMMAND core_tests)
endif()

message(STATUS "Place plugins next to the executable under bin/plugins.")
