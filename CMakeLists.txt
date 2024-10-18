CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(pstress)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
enable_testing()

find_package(Threads REQUIRED)
find_package(libmysqlclient REQUIRED)
find_package(libpqxx REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(CLI11 REQUIRED)
find_package(Catch2 REQUIRED)
find_package(Git)

# Official release lack a required feature, keep it in 3rd party folder
ADD_SUBDIRECTORY(src/third_party/inih++)

INCLUDE(PQSetupCompiler)

INCLUDE(PQCPack)
INCLUDE(PQFindRevision)

add_subdirectory(src)
add_subdirectory(test/unit)
