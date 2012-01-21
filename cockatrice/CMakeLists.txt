CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(cockatrice)

SET(cockatrice_SOURCES
 src/abstractcounter.cpp 
 src/counter_general.cpp 
 src/dlg_creategame.cpp 
 src/dlg_connect.cpp 
 src/dlg_create_token.cpp 
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
 src/window_deckeditor.cpp 
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
 src/tab_supervisor.cpp 
 src/tab_admin.cpp 
 src/tab_userlists.cpp 
 src/chatview.cpp 
 src/userlist.cpp 
 src/userinfobox.cpp 
 src/remotedecklist_treewidget.cpp 
 src/deckview.cpp 
 src/playerlistwidget.cpp 
 src/pixmapgenerator.cpp 
 src/settingscache.cpp 
 src/localserver.cpp 
 src/localserverinterface.cpp 
 src/localclient.cpp 
 src/priceupdater.cpp 
 src/soundengine.cpp 
)
SET(cockatrice_HEADERS
 src/abstractcounter.h 
 src/counter_general.h 
 src/dlg_creategame.h 
 src/dlg_connect.h 
 src/dlg_create_token.h 
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
 src/window_deckeditor.h 
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
 src/tab_supervisor.h 
 src/tab_admin.h 
 src/tab_userlists.h 
 src/chatview.h 
 src/userlist.h 
 src/userinfobox.h 
 src/remotedecklist_treewidget.h 
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

set(cockatrice_RESOURCES cockatrice.qrc)
set(cockatrice_TS
        translations/cockatrice_cs.ts
        translations/cockatrice_de.ts
        translations/cockatrice_en.ts
        translations/cockatrice_es.ts
        translations/cockatrice_fr.ts
        translations/cockatrice_it.ts
        translations/cockatrice_ja.ts
        translations/cockatrice_pl.ts
        translations/cockatrice_pt-br.ts
        translations/cockatrice_pt.ts
        translations/cockatrice_ru.ts
        translations/cockatrice_sk.ts
)	

if(WIN32)
  set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
  set(OSX_ICON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
  set_source_files_properties(${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set( cockatrice_SOURCES ${cockatrice_SOURCES} ${OSX_ICON_FILES} )
ENDIF(APPLE)

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSCRIPT TRUE)
SET(QT_USE_QTMULTIMEDIA TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
if (NOT QT_QTMULTIMEDIA_FOUND)
	FIND_PACKAGE(QtMobility REQUIRED)
endif (NOT QT_QTMULTIMEDIA_FOUND)
FIND_PACKAGE(Protobuf REQUIRED)
set(CMAKE_BUILD_TYPE Release)

QT4_WRAP_CPP(cockatrice_HEADERS_MOC ${cockatrice_HEADERS})
QT4_ADD_TRANSLATION(${cockatrice_QM} ${cockatrice_TS})
QT4_ADD_RESOURCES(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)
INCLUDE_DIRECTORIES(${QT_MOBILITY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_MOBILITY_MULTIMEDIAKIT_INCLUDE_DIR})

ADD_EXECUTABLE(cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_SOURCES} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC} ${cockatrice_HEADERS_MOC})
TARGET_LINK_LIBRARIES(cockatrice cockatrice_common ${QT_LIBRARIES} ${QT_MOBILITY_MULTIMEDIAKIT_LIBRARY} ${PROTOBUF_LIBRARIES})

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/cockatrice DESTINATION bin)
