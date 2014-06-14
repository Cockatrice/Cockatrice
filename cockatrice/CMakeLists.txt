# CMakeLists for cockatrice directory
#
# provides the cockatrice binary

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
    src/keysignals.cpp
    src/gameview.cpp 
    src/gameselector.cpp 
    src/decklistmodel.cpp 
    src/deck_loader.cpp 
    src/dlg_load_deck_from_clipboard.cpp 
    src/dlg_load_remote_deck.cpp 
    src/cardinfowidget.cpp 
    src/cardframe.cpp
    src/cardinfopicture.cpp
    src/cardinfotext.cpp
    src/filterbuilder.cpp
    src/cardfilter.cpp
    src/filtertreemodel.cpp
    src/filtertree.cpp
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
    src/pending_command.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
)

if (UNIX AND NOT APPLE)
    set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -DTRANSLATION_PATH=\\"${CMAKE_INSTALL_PREFIX}/share/cockatrice/translations\\")
endif (UNIX AND NOT APPLE)

set(cockatrice_RESOURCES cockatrice.qrc)
set(cockatrice_TS
#    translations/cockatrice_cs.ts
    translations/cockatrice_de.ts
    translations/cockatrice_en.ts
    translations/cockatrice_es.ts
#    translations/cockatrice_fr.ts
    translations/cockatrice_it.ts
    translations/cockatrice_ja.ts
#    translations/cockatrice_pl.ts
#    translations/cockatrice_pt-br.ts
    translations/cockatrice_pt.ts
#    translations/cockatrice_ru.ts
#    translations/cockatrice_sk.ts
    translations/cockatrice_sv.ts
#    translations/cockatrice_zh_CN.ts
)

if(WIN32)
    set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(cockatrice_SOURCES ${cockatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

if (NOT QT_QTMULTIMEDIA_FOUND)
    FIND_PACKAGE(QtMobility REQUIRED)
endif (NOT QT_QTMULTIMEDIA_FOUND)

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTMULTIMEDIA TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)

# Declare path variables
set(ICONDIR share/icons CACHE STRING "icon dir")
set(DESKTOPDIR share/applications CACHE STRING "desktop file destination")

# Let cmake chew Qt4's translations and resource files
# Note: header files are MOC-ed automatically by cmake
QT4_ADD_TRANSLATION(cockatrice_QM ${cockatrice_TS})
QT4_ADD_RESOURCES(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})

# Include directories
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${QT_MOBILITY_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_MOBILITY_MULTIMEDIAKIT_INCLUDE_DIR})

# Build cockatrice binary and link it
ADD_EXECUTABLE(cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_SOURCES} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC} ${cockatrice_MOC_SRCS})
TARGET_LINK_LIBRARIES(cockatrice cockatrice_common ${QT_LIBRARIES} ${QT_MOBILITY_MULTIMEDIAKIT_LIBRARY})


if(UNIX)
    if(APPLE)
        INSTALL(TARGETS cockatrice BUNDLE DESTINATION ./)
        INSTALL(FILES ${cockatrice_QM} DESTINATION ./cockatrice.app/Contents/Resources/translations)
    else()
        # Assume linux
        INSTALL(TARGETS cockatrice RUNTIME DESTINATION bin/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cockatrice.desktop DESTINATION ${DESKTOPDIR})
        INSTALL(FILES ${cockatrice_QM} DESTINATION share/cockatrice/translations)
    endif()
elseif(WIN32)
        INSTALL(TARGETS cockatrice RUNTIME DESTINATION ./)
        INSTALL(FILES ${cockatrice_QM} DESTINATION ./translations)
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp ${CMAKE_CURRENT_BINARY_DIR}/version_string.h
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/getversion.cmake
)

if(APPLE)
    set(plugin_dest_dir ./cockatrice.app/Contents/Plugins)
    set(qtconf_dest_dir ./cockatrice.app/Contents/Resources)

    # note: no codecs in qt5
    # note: phonon_backend => mediaservice
    # note: needs platform on osx

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/.*_debug\\.dylib")
    else()
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/[^_]*\\.dylib")
    endif()

    install(CODE "
        file(WRITE \"${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins\")
        " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"${plugin_dest_dir}/*.dylib\")
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/cockatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()
