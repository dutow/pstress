CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
PROJECT(pstress)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
enable_testing()

option(WITH_ASAN "Build with address sanitizer")
option(WITH_UBSAN "Build with undefined behavior sanitizer")

if (WITH_ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif ()

if (WITH_UBSAN)
	add_compile_options(-fsanitize=undefined)
	add_link_options(-fsanitize=undefined)
endif ()

if (WITH_TSAN)
	add_compile_options(-fsanitize=thread)
	add_link_options(-fsanitize=thread)
endif ()

if (WITH_MSAN)
	add_compile_options(-fsanitize=memory)
	add_link_options(-fsanitize=memory)
endif ()

if (WITH_ASAN OR WITH_UBSAN OR WITH_TSAN OR WITH_MSAN)
	add_compile_options(-fno-omit-frame-pointer)
endif()

find_package(Threads REQUIRED)
find_package(libmysqlclient REQUIRED)
find_package(libpqxx REQUIRED)
find_package(CLI11 REQUIRED)
find_package(Catch2 REQUIRED)
find_package(Boost REQUIRED)
find_package(reflectcpp REQUIRED)
find_package(Git REQUIRED)
find_package(Sol2 REQUIRED)
find_package(spdlog REQUIRED)

INCLUDE(PQSetupCompiler)

INCLUDE(PQCPack)
INCLUDE(PQFindRevision)

add_subdirectory(libpstress)
add_subdirectory(pstress)
