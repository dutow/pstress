CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
PROJECT(pstress)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
enable_testing()

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
