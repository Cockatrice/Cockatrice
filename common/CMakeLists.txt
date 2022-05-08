# CMakeLists for common directory
#
# provides the common library

add_subdirectory(pb)

set(common_SOURCES
    debug_pb_message.cpp
    decklist.cpp
    expression.cpp
    featureset.cpp
    get_pb_extension.cpp
    passwordhasher.cpp
    rng_abstract.cpp
    rng_sfmt.cpp
    server.cpp
    server_abstractuserinterface.cpp
    server_arrow.cpp
    server_arrowtarget.h
    server_card.cpp
    server_cardzone.cpp
    server_counter.cpp
    server_database_interface.cpp
    server_game.cpp
    server_player.cpp
    server_protocolhandler.cpp
    server_remoteuserinterface.cpp
    server_response_containers.cpp
    server_room.cpp
    serverinfo_user_container.cpp
    sfmt/SFMT.c
)

set(ORACLE_LIBS)

include_directories(pb)
include_directories(sfmt)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(cockatrice_common ${common_SOURCES} ${common_MOC_SRCS})
target_link_libraries(cockatrice_common PUBLIC cockatrice_protocol)
