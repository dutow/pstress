
SET(UNITTEST_SOURCES
    main.cpp
    metadata_test.cpp
    options_test.cpp
)

add_executable(pstress-unit ${UNITTEST_SOURCES})
target_link_libraries(pstress-unit
    Catch2::Catch2 libpstress
)

add_test(NAME pstress-unit COMMAND sh -c "mkdir -p ${CMAKE_BINARY_DIR}/test-reports && $<TARGET_FILE:pstress-unit> --reporter=JUnit >${CMAKE_BINARY_DIR}/test-reports/pstress-unit.xml")