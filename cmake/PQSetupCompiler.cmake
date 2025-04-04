SET(CMAKE_CXX_STANDARD 20)

#
IF((CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86|AMD64") AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
  SET(ARCH "x86")
ELSEIF((CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64") AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  SET(ARCH "x86_64")
ELSEIF((CMAKE_SYSTEM_PROCESSOR MATCHES "i386") AND (CMAKE_SIZEOF_VOID_P EQUAL 8) AND (APPLE))
  # Mac is weird like that.
  SET(ARCH "x86_64")
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm*")
  SET(ARCH "ARM")
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
  SET(ARCH "sparc")
ENDIF()
#
MESSAGE(STATUS "Architecture is ${ARCH}")
#
OPTION(STRICT_FLAGS "Turn on a lot of compiler warnings" ON)
OPTION(ASAN "Turn ON Address sanitizer feature" OFF)
OPTION(DEBUG "Add debug info for GDB" OFF)
OPTION(STATIC_LIBRARY "Statically compile MySQL library into PStress" ON)
OPTION(STRICT_CPU "Strictly bind the binary to current CPU" OFF)
#
# Debug Release RelWithDebInfo MinSizeRel
IF(CMAKE_BUILD_TYPE STREQUAL "")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF()
#
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  ADD_DEFINITIONS(-ggdb3)
ENDIF()
##
IF(STRICT_CPU)
  ADD_DEFINITIONS(-march=native -mtune=generic)
ENDIF()
#
IF(STRICT_FLAGS)
  ADD_DEFINITIONS(-Wall -Werror -Wextra -pedantic-errors -Wmissing-declarations)
ENDIF ()
##
IF(ASAN)
  # doesn't work with GCC < 4.8
  ADD_DEFINITIONS(-fsanitize=address)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
ENDIF()

#ADD_DEFINITIONS(-stdlib=libc++ -fexperimental-library) # temporary
