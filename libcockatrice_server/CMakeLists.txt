set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/server/game/server_abstract_participant.h
    libcockatrice/server/game/server_abstract_player.h
    libcockatrice/server/game/server_arrow.h
    libcockatrice/server/game/server_arrowtarget.h
    libcockatrice/server/game/server_card.h
    libcockatrice/server/game/server_cardzone.h
    libcockatrice/server/game/server_counter.h
    libcockatrice/server/game/server_game.h
    libcockatrice/server/game/server_player.h
    libcockatrice/server/game/server_spectator.h
    libcockatrice/server/debug_pb_message.h
    libcockatrice/server/get_pb_extension.h
    libcockatrice/server/server.h
    libcockatrice/server/server_abstractuserinterface.h
    libcockatrice/server/server_database_interface.h
    libcockatrice/server/server_protocolhandler.h
    libcockatrice/server/server_remoteuserinterface.h
    libcockatrice/server/server_response_containers.h
    libcockatrice/server/server_room.h
    libcockatrice/server/serverinfo_user_container.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_server STATIC
  ${MOC_SOURCES}
  libcockatrice/server/debug_pb_message.cpp
  libcockatrice/server/game/server_abstract_participant.cpp
  libcockatrice/server/game/server_abstract_player.cpp
  libcockatrice/server/game/server_arrow.cpp
  libcockatrice/server/game/server_arrowtarget.cpp
  libcockatrice/server/game/server_card.cpp
  libcockatrice/server/game/server_cardzone.cpp
  libcockatrice/server/game/server_counter.cpp
  libcockatrice/server/game/server_game.cpp
  libcockatrice/server/game/server_player.cpp
  libcockatrice/server/game/server_spectator.cpp
  libcockatrice/server/get_pb_extension.cpp
  libcockatrice/server/server.cpp
  libcockatrice/server/server_abstractuserinterface.cpp
  libcockatrice/server/server_database_interface.cpp
  libcockatrice/server/server_protocolhandler.cpp
  libcockatrice/server/server_remoteuserinterface.cpp
  libcockatrice/server/server_response_containers.cpp
  libcockatrice/server/server_room.cpp
  libcockatrice/server/serverinfo_user_container.cpp
)

add_dependencies(libcockatrice_server libcockatrice_protocol)

target_include_directories(libcockatrice_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Make cockatrice_server depend on cockatrice_protocol
target_link_libraries(
    libcockatrice_server PUBLIC libcockatrice_protocol libcockatrice_utility libcockatrice_rng
                              libcockatrice_deck_list ${COCKATRICE_QT_MODULES}
)
