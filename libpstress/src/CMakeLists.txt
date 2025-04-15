
set(LIBRARY_SOURCES
    action/action.cpp
    action/action_registry.cpp
    action/custom.cpp
    action/ddl.cpp
    action/dml.cpp
    process/postgres.cpp
    random.cpp
    metadata.cpp
    workload.cpp
    sql_variant/generic.cpp
    #sql_variant/mysql.cpp
    sql_variant/postgresql.cpp
    sql_variant/sql_variant.cpp
)

ADD_LIBRARY(libpstress STATIC ${LIBRARY_SOURCES})
TARGET_INCLUDE_DIRECTORIES(libpstress PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include/")
TARGET_LINK_LIBRARIES(libpstress 
    #libmysqlclient::libmysqlclient
    libpqxx::pqxx
    CLI11::CLI11
    boost::boost
    reflectcpp::reflectcpp
    spdlog::spdlog
)
SET_PROPERTY(TARGET libpstress PROPERTY CXX_STANDARD 23)
