CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(pstress)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(libmysqlclient REQUIRED)
FIND_PACKAGE(RapidJSON REQUIRED)
find_package(Git)

# Official release lack a required feature, keep it in 3rd party folder
ADD_SUBDIRECTORY(src/third_party/inih++)

INCLUDE(PQSetupCompiler)

INCLUDE(PQCPack)
INCLUDE(PQFindRevision)

ADD_SUBDIRECTORY(src)
