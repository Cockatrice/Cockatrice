set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    local_client.h
)

set(SOURCES
    local_client.cpp
)

if (Qt6_FOUND)
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif (Qt5_FOUND)
    qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif ()

add_library(
    libcockatrice_network_client_local STATIC
    ${MOC_SOURCES}
    ${SOURCES}
)

# add_dependencies(libcockatrice_server libcockatrice_protocol_wrapper)

target_include_directories(libcockatrice_network_client_local PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    libcockatrice_network_client_local PUBLIC ${COCKATRICE_QT_MODULES}
)
