
SET(PSTRESS_SOURCES
  pstress.cpp
  help.cpp
  node.cpp
  thread.cpp
  random_test.cpp
  sql_variant/mysql.cpp
)

ADD_EXECUTABLE(pstress ${PSTRESS_SOURCES})
TARGET_INCLUDE_DIRECTORIES(pstress PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
INSTALL(TARGETS pstress DESTINATION bin)
TARGET_LINK_LIBRARIES(pstress libmysqlclient::libmysqlclient)
TARGET_LINK_LIBRARIES(pstress inih++ rapidjson)

FILE(COPY
       grammar.sql
       DESTINATION .)
INSTALL(FILES grammar.sql DESTINATION bin)
