
set(HEADERS         config.h
                    environmentoptions.h
                    gl_const.h
                    ilogger.h
                    map.h
                    mission.h
                    node.h
                    search.h
                    searchresult.h
                    tinyxml2.h
                    xmllogger.h)

set(SOURCES         config.cpp
                    environmentoptions.cpp
                    map.cpp
                    mission.cpp
                    search.cpp
                    tinyxml2.cpp
                    xmllogger.cpp)

add_executable(ARAStarPathPlanning ${HEADERS} ${SOURCES} main.cpp)

add_library(PPLIB ${HEADERS} ${SOURCES})

install(TARGETS ARAStarPathPlanning
        CONFIGURATIONS Debug
        DESTINATION Bin/Debug)

install(TARGETS ARAStarPathPlanning
        CONFIGURATIONS Release
        DESTINATION Bin/Release)
