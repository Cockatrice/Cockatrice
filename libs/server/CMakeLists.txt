set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    include/server/game/server_abstract_participant.h
    include/server/game/server_arrow.h
    include/server/game/server_arrowtarget.h
    include/server/game/server_card.h
    include/server/game/server_cardzone.h
    include/server/game/server_counter.h
    include/server/game/server_game.h
    include/server/game/server_player.h
    include/server/game/server_spectator.h
    include/server/debug_pb_message.h
    include/server/get_pb_extension.h
    include/server/server.h
    include/server/server_abstractuserinterface.h
    include/server/server_database_interface.h
    include/server/server_protocolhandler.h
    include/server/server_remoteuserinterface.h
    include/server/server_response_containers.h
    include/server/server_room.h
    include/server/serverinfo_user_container.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  cockatrice_server STATIC
  ${MOC_SOURCES}
  src/debug_pb_message.cpp
  src/game/server_abstract_participant.cpp
  src/game/server_arrow.cpp
  src/game/server_arrowtarget.cpp
  src/game/server_card.cpp
  src/game/server_cardzone.cpp
  src/game/server_counter.cpp
  src/game/server_game.cpp
  src/game/server_player.cpp
  src/game/server_spectator.cpp
  src/get_pb_extension.cpp
  src/server.cpp
  src/server_abstractuserinterface.cpp
  src/server_database_interface.cpp
  src/server_protocolhandler.cpp
  src/server_remoteuserinterface.cpp
  src/server_response_containers.cpp
  src/server_room.cpp
  src/serverinfo_user_container.cpp
)

add_dependencies(cockatrice_server cockatrice_protocol_wrapper)

target_include_directories(
  cockatrice_server
  PUBLIC include
  PUBLIC ${CMAKE_SOURCE_DIR}/common
)

# Make cockatrice_server depend on cockatrice_protocol
target_link_libraries(
  cockatrice_server PUBLIC cockatrice_common cockatrice_protocol_wrapper utility cockatrice_rng cockatrice_deck_list
                           ${COCKATRICE_QT_MODULES}
)
