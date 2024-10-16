include_directories(../include)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
add_library(inih++ STATIC ini.c INIReader.cpp)
target_include_directories(inih++ PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")