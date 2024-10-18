
set(LIBRARY_SOURCES
  options.cpp
  sql_variant/generic.cpp
  sql_variant/mysql.cpp
  sql_variant/postgresql.cpp
  sql_variant/sql_variant.cpp
)

SET(PSTRESS_SOURCES
  pstress.cpp
  help.cpp
  node.cpp
  thread.cpp
  random_test.cpp
)

add_library(libpstress STATIC ${LIBRARY_SOURCES})
TARGET_INCLUDE_DIRECTORIES(libpstress PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(libpstress libmysqlclient::libmysqlclient libpqxx::pqxx CLI11::CLI11)

ADD_EXECUTABLE(pstress ${PSTRESS_SOURCES})
TARGET_INCLUDE_DIRECTORIES(pstress PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(pstress inih++ rapidjson libpstress)


INSTALL(TARGETS pstress DESTINATION bin)

FILE(COPY
       grammar.sql
       DESTINATION .)
INSTALL(FILES grammar.sql DESTINATION bin)
