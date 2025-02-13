cmake_minimum_required(VERSION 3.11)

project(cpp-search-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Основные исходники
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)

# Создаем библиотеку из основного кода
add_library(search-server-lib ${SOURCES} ${HEADERS})

target_compile_options(search-server-lib PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Исполняемый файл поискового сервера
add_executable(search-server src/main.cpp)  # main.cpp - точка входа
target_link_libraries(search-server PRIVATE search-server-lib)

# Добавляем тесты только в Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)

    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE search-server-lib)

    message(STATUS "Building tests (Debug mode)")
endif()
