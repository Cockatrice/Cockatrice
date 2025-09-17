# CMakeLists for common directory
#
# provides the common library

add_subdirectory(pb)

set(common_SOURCES
    abstract_deck_list_card_node.cpp
    abstract_deck_list_node.cpp
    debug_pb_message.cpp
    deck_list_card_node.cpp
    deck_list.cpp
    expression.cpp
    featureset.cpp
    get_pb_extension.cpp
    inner_deck_list_node.cpp
    passwordhasher.cpp
    rng_abstract.cpp
    rng_sfmt.cpp
    server/game/server_arrow.cpp
    server/game/server_arrowtarget.cpp
    server/game/server_card.cpp
    server/game/server_cardzone.cpp
    server/game/server_counter.cpp
    server/game/server_game.cpp
    server/game/server_player.cpp
    server/server.cpp
    server/server_abstractuserinterface.cpp
    server/server_database_interface.cpp
    server/server_protocolhandler.cpp
    server/server_remoteuserinterface.cpp
    server/server_response_containers.cpp
    server/server_room.cpp
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
