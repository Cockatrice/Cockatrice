set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    game/server_abstract_participant.h
    game/server_abstract_player.h
    game/server_arrow.h
    game/server_arrowtarget.h
    game/server_card.h
    game/server_cardzone.h
    game/server_counter.h
    game/server_game.h
    game/server_player.h
    game/server_spectator.h
    server.h
    server_abstractuserinterface.h
    server_database_interface.h
    server_protocolhandler.h
    server_remoteuserinterface.h
    server_response_containers.h
    server_room.h
    serverinfo_user_container.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_network_server_remote STATIC
  ${MOC_SOURCES}
  game/server_abstract_participant.cpp
  game/server_abstract_player.cpp
  game/server_arrow.cpp
  game/server_arrowtarget.cpp
  game/server_card.cpp
  game/server_cardzone.cpp
  game/server_counter.cpp
  game/server_game.cpp
  game/server_player.cpp
  game/server_spectator.cpp
  server.cpp
  server_abstractuserinterface.cpp
  server_database_interface.cpp
  server_protocolhandler.cpp
  server_remoteuserinterface.cpp
  server_response_containers.cpp
  server_room.cpp
  serverinfo_user_container.cpp
)

add_dependencies(libcockatrice_network_server_remote libcockatrice_protocol)

target_include_directories(libcockatrice_network_server_remote PUBLIC .)

# Make cockatrice_server depend on cockatrice_protocol
target_link_libraries(
  libcockatrice_network_server_remote PUBLIC libcockatrice_protocol libcockatrice_utility libcockatrice_rng
                                             libcockatrice_deck_list ${COCKATRICE_QT_MODULES}
)
