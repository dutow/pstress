
SET(PSTRESS_SOURCES
    main.cpp
)

ADD_EXECUTABLE(pstress ${PSTRESS_SOURCES})
TARGET_LINK_LIBRARIES(pstress
    libpstress
    sol2::sol2
    nlohmann_json::nlohmann_json
    toml-lua
)

INSTALL(TARGETS pstress DESTINATION bin)
