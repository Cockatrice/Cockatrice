CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(cockatrice)

SET(cockatrice_SOURCES
 src/abstractcounter.cpp 
 src/counter_general.cpp 
 src/dlg_creategame.cpp 
 src/dlg_filter_games.cpp
 src/dlg_connect.cpp 
 src/dlg_create_token.cpp 
 src/dlg_edit_tokens.cpp
 src/abstractclient.cpp 
 src/remoteclient.cpp 
 src/main.cpp 
 src/window_main.cpp 
 src/gamesmodel.cpp 
 src/player.cpp 
 src/playertarget.cpp 
 src/cardzone.cpp 
 src/selectzone.cpp 
 src/cardlist.cpp 
 src/abstractcarditem.cpp 
 src/carditem.cpp 
 src/tablezone.cpp 
 src/handzone.cpp 
 src/handcounter.cpp 
 src/carddatabase.cpp 
 src/gameview.cpp 
 src/gameselector.cpp 
 src/decklistmodel.cpp 
 src/deck_loader.cpp 
 src/dlg_load_deck_from_clipboard.cpp 
 src/dlg_load_remote_deck.cpp 
 src/cardinfowidget.cpp 
 src/messagelogwidget.cpp 
 src/zoneviewzone.cpp 
 src/zoneviewwidget.cpp 
 src/pilezone.cpp 
 src/stackzone.cpp 
 src/carddragitem.cpp 
 src/carddatabasemodel.cpp 
 src/setsmodel.cpp 
 src/window_sets.cpp 
 src/abstractgraphicsitem.cpp 
 src/abstractcarddragitem.cpp 
 src/dlg_settings.cpp 
 src/dlg_cardsearch.cpp 
 src/phasestoolbar.cpp 
 src/gamescene.cpp 
 src/arrowitem.cpp 
 src/arrowtarget.cpp 
 src/tab.cpp 
 src/tab_server.cpp 
 src/tab_room.cpp 
 src/tab_message.cpp 
 src/tab_game.cpp 
 src/tab_deck_storage.cpp 
 src/tab_replays.cpp 
 src/tab_supervisor.cpp 
 src/tab_admin.cpp 
 src/tab_userlists.cpp 
 src/tab_deck_editor.cpp
 src/replay_timeline_widget.cpp
 src/deckstats_interface.cpp
 src/chatview.cpp 
 src/userlist.cpp 
 src/userinfobox.cpp 
 src/user_context_menu.cpp
 src/remotedecklist_treewidget.cpp 
 src/remotereplaylist_treewidget.cpp 
 src/deckview.cpp 
 src/playerlistwidget.cpp 
 src/pixmapgenerator.cpp 
 src/settingscache.cpp 
 src/localserver.cpp 
 src/localserverinterface.cpp 
 src/localclient.cpp 
 src/priceupdater.cpp 
 src/qt-json/json.cpp
 src/soundengine.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
)
SET(cockatrice_HEADERS
 src/abstractcounter.h 
 src/counter_general.h 
 src/dlg_creategame.h 
 src/dlg_filter_games.h 
 src/dlg_connect.h 
 src/dlg_create_token.h 
 src/dlg_edit_tokens.h
 src/gamesmodel.h 
 src/abstractclient.h 
 src/remoteclient.h 
 src/window_main.h 
 src/cardzone.h 
 src/selectzone.h 
 src/player.h 
 src/playertarget.h 
 src/abstractcarditem.h 
 src/carditem.h 
 src/tablezone.h 
 src/handzone.h 
 src/handcounter.h 
 src/carddatabase.h 
 src/gameview.h 
 src/gameselector.h 
 src/decklistmodel.h 
 src/deck_loader.h 
 src/dlg_load_deck_from_clipboard.h 
 src/dlg_load_remote_deck.h 
 src/cardinfowidget.h 
 src/messagelogwidget.h 
 src/zoneviewzone.h 
 src/zoneviewwidget.h 
 src/pilezone.h 
 src/stackzone.h 
 src/carddragitem.h 
 src/carddatabasemodel.h 
 src/setsmodel.h 
 src/window_sets.h 
 src/abstractgraphicsitem.h 
 src/abstractcarddragitem.h 
 src/dlg_settings.h 
 src/dlg_cardsearch.h 
 src/phasestoolbar.h 
 src/gamescene.h 
 src/arrowitem.h 
 src/arrowtarget.h 
 src/tab.h 
 src/tab_server.h 
 src/tab_room.h 
 src/tab_message.h 
 src/tab_game.h 
 src/tab_deck_storage.h 
 src/tab_replays.h 
 src/tab_supervisor.h 
 src/tab_admin.h 
 src/tab_userlists.h 
 src/tab_deck_editor.h
 src/replay_timeline_widget.h
 src/deckstats_interface.h
 src/chatview.h 
 src/userlist.h 
 src/userinfobox.h 
 src/user_context_menu.h
 src/remotedecklist_treewidget.h 
 src/remotereplaylist_treewidget.h 
 src/deckview.h 
 src/playerlistwidget.h 
 src/settingscache.h 
 src/localserver.h 
 src/localserverinterface.h 
 src/localclient.h 
 src/priceupdater.h 
 src/soundengine.h 
 src/pending_command.h 
)

if (UNIX)
	set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -DTRANSLATION_PATH=\\"${CMAKE_INSTALL_PREFIX}/share/cockatrice/translations\\")
endif (UNIX)
if (WIN32)
	set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -DTRANSLATION_PATH=\\"translations\\")
endif (WIN32)
if (APPLE)
	set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -DTRANSLATION_PATH=\\"../../../translations\\")
endif (APPLE)

set(cockatrice_RESOURCES cockatrice.qrc)
set(cockatrice_TS
#        translations/cockatrice_cs.ts
        translations/cockatrice_de.ts
        translations/cockatrice_en.ts
        translations/cockatrice_es.ts
#        translations/cockatrice_fr.ts
        translations/cockatrice_it.ts
        translations/cockatrice_ja.ts
#        translations/cockatrice_pl.ts
#        translations/cockatrice_pt-br.ts
        translations/cockatrice_pt.ts
#        translations/cockatrice_ru.ts
#        translations/cockatrice_sk.ts
	translations/cockatrice_sv.ts
#	translations/cockatrice_zh_CN.ts
)

if(WIN32)
  set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(cockatrice_SOURCES ${cockatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTMULTIMEDIA TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
if (NOT QT_QTMULTIMEDIA_FOUND)
	FIND_PACKAGE(QtMobility REQUIRED)
endif (NOT QT_QTMULTIMEDIA_FOUND)
FIND_PACKAGE(Protobuf REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")

QT4_WRAP_CPP(cockatrice_HEADERS_MOC ${cockatrice_HEADERS})
QT4_ADD_TRANSLATION(cockatrice_QM ${cockatrice_TS})
QT4_ADD_RESOURCES(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${QT_MOBILITY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_MOBILITY_MULTIMEDIAKIT_INCLUDE_DIR})

ADD_EXECUTABLE(cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_SOURCES} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC} ${cockatrice_HEADERS_MOC})
TARGET_LINK_LIBRARIES(cockatrice cockatrice_common ${QT_LIBRARIES} ${QT_MOBILITY_MULTIMEDIAKIT_LIBRARY})

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cockatrice DESTINATION bin)
if (NOT WIN32 AND NOT APPLE)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.png DESTINATION share/icons/hicolor/48x48/apps)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.svg DESTINATION share/icons/hicolor/scalable/apps)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cockatrice.desktop DESTINATION share/applications)
	INSTALL(FILES ${cockatrice_QM} DESTINATION share/cockatrice/translations)
ENDIF(NOT WIN32 AND NOT APPLE)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp ${CMAKE_CURRENT_BINARY_DIR}/version_string.h
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/getversion.cmake
)

