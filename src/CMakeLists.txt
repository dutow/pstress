
SET(PSTRESS_SOURCES
  pstress.cpp
  help.cpp
  node.cpp
  thread.cpp
  random_test.cpp
)

FUNCTION(add_pstress_favor NAME EXT)
  ADD_EXECUTABLE(pstress-${EXT} ${PSTRESS_SOURCES})
  INSTALL(TARGETS pstress-${EXT} DESTINATION bin)
  TARGET_LINK_LIBRARIES(pstress-${EXT} libmysqlclient::libmysqlclient)
  TARGET_LINK_LIBRARIES(pstress-${EXT} inih++ rapidjson)
ENDFUNCTION()

add_pstress_favor("MySQL" "mds")
add_pstress_favor("MariaDB" "md")
add_pstress_favor("Percona Server" "ps")
add_pstress_favor("Percona-XtraDB-Cluster" "pxc")

FILE(COPY
       grammar.sql
       DESTINATION .)
INSTALL(FILES grammar.sql DESTINATION bin)
