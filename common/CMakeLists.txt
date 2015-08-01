# CMakeLists for common directory
#
# provides the common library

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
add_subdirectory(pb)

SET(common_SOURCES
    decklist.cpp
    get_pb_extension.cpp
    rng_abstract.cpp
    rng_sfmt.cpp
    server.cpp
    server_abstractuserinterface.cpp
    server_arrow.cpp
    server_arrowtarget.h
    server_card.cpp
    server_cardzone.cpp
    server_counter.cpp
    server_game.cpp
    server_database_interface.cpp
    server_settings_interface.cpp
    server_player.cpp
    server_protocolhandler.cpp
    server_remoteuserinterface.cpp
    server_response_containers.cpp
    server_room.cpp
    serverinfo_user_container.cpp
    sfmt/SFMT.c
)

set(ORACLE_LIBS)

# Qt4 stuff
if(Qt4_FOUND)
    # Include directories
    INCLUDE(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES(pb)
INCLUDE_DIRECTORIES(sfmt)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_library(cockatrice_common ${common_SOURCES} ${common_MOC_SRCS})
target_link_libraries(cockatrice_common cockatrice_protocol)
