CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
add_subdirectory(pb)

SET(common_SOURCES
	decklist.cpp
	get_pb_extension.cpp
	rng_abstract.cpp
	rng_qt.cpp
	rng_sfmt.cpp
	server.cpp
	server_abstractuserinterface.cpp
	server_arrow.cpp
	server_card.cpp
	server_cardzone.cpp
	server_counter.cpp
	server_game.cpp
	server_player.cpp
	server_protocolhandler.cpp
	server_remoteuserinterface.cpp
	server_response_containers.cpp
	server_room.cpp
	serverinfo_user_container.cpp
	sfmt/SFMT.c
)
SET(common_HEADERS
	decklist.h
	rng_abstract.h
	rng_qt.h
	rng_sfmt.h
	server.h
	server_arrowtarget.h
	server_card.h
	server_database_interface.h
	server_game.h
	server_player.h
	server_protocolhandler.h
	server_room.h
)

FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Protobuf REQUIRED)

QT4_WRAP_CPP(common_HEADERS_MOC ${common_HEADERS})
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(pb)
INCLUDE_DIRECTORIES(sfmt)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_library(cockatrice_common ${common_SOURCES} ${common_HEADERS_MOC})
target_link_libraries(cockatrice_common cockatrice_protocol)

