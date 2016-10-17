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
    src/dlg_edit_avatar.cpp
    src/dlg_edit_password.cpp
    src/dlg_edit_tokens.cpp
    src/dlg_edit_user.cpp
    src/dlg_register.cpp 
    src/dlg_update.cpp
    src/dlg_viewlog.cpp
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
    src/tab_logs.cpp
    src/replay_timeline_widget.cpp
    src/deckstats_interface.cpp
    src/tappedout_interface.cpp
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
    src/thememanager.cpp 
    src/localserver.cpp 
    src/localserverinterface.cpp 
    src/localclient.cpp 
    src/priceupdater.cpp 
    src/qt-json/json.cpp
    src/soundengine.cpp
    src/pending_command.cpp
    src/pictureloader.cpp
    src/shortcutssettings.cpp
    src/sequenceEdit/sequenceedit.cpp
    src/sequenceEdit/shortcutstab.cpp
    src/lineeditcompleter.cpp
    src/settings/settingsmanager.cpp
    src/settings/carddatabasesettings.cpp
    src/settings/serverssettings.cpp
    src/settings/messagesettings.cpp
    src/settings/gamefilterssettings.cpp
    src/settings/layoutssettings.cpp
    src/update_checker.cpp
    src/update_downloader.cpp
    src/logger.cpp
    ${VERSION_STRING_CPP}
)

add_subdirectory(sounds)
add_subdirectory(themes)

set(cockatrice_RESOURCES cockatrice.qrc)

IF(UPDATE_TRANSLATIONS)
    FILE(GLOB_RECURSE translate_cockatrice_SRCS ${CMAKE_SOURCE_DIR}/cockatrice/src/*.cpp ${CMAKE_SOURCE_DIR}/cockatrice/src/*.h)
    FILE(GLOB_RECURSE translate_common_SRCS ${CMAKE_SOURCE_DIR}/common/*.cpp ${CMAKE_SOURCE_DIR}/common/*.h)
    SET(translate_SRCS ${translate_cockatrice_SRCS} ${translate_common_SRCS})
    SET(cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/cockatrice_en.ts")
ELSE()
    FILE(GLOB cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
ENDIF(UPDATE_TRANSLATIONS)

if(WIN32)
    set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(cockatrice_SOURCES ${cockatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

set(COCKATRICE_LIBS)

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND COCKATRICE_LIBS Widgets)

    # QtConcurrent
    find_package(Qt5Concurrent)
    if(Qt5Concurrent_FOUND)
        include_directories(${Qt5Concurrent_INCLUDE_DIRS})
        list(APPEND ORACLE_LIBS Concurrent)
    endif()

    # QtNetwork
    find_package(Qt5Network)
    if(Qt5Network_FOUND)
        include_directories(${Qt5Network_INCLUDE_DIRS})
        list(APPEND COCKATRICE_LIBS Network)
    endif()

    # QtMultimedia
    find_package(Qt5Multimedia)
    if(Qt5Multimedia_FOUND)
        include_directories(${Qt5Multimedia_INCLUDE_DIRS})
        list(APPEND COCKATRICE_LIBS Multimedia)
    endif()

    # QtPrinter
    find_package(Qt5PrintSupport)
    if(Qt5PrintSupport_FOUND)
        include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
        list(APPEND COCKATRICE_LIBS PrintSupport)
    endif()

    # QtSvg
    find_package(Qt5Svg)
    if(Qt5Svg_FOUND)
        include_directories(${Qt5Svg_INCLUDE_DIRS})
        list(APPEND COCKATRICE_LIBS Svg)
    endif()

    # Qt5LinguistTools
    find_package(Qt5LinguistTools)
    if(Qt5LinguistTools_FOUND)
        include_directories(${Qt5LinguistTools_INCLUDE_DIRS})
        list(APPEND COCKATRICE_LIBS LinguistTools)
    endif()

    # Let cmake chew Qt5's translations and resource files
    # Note: header files are MOC-ed automatically by cmake
    IF(UPDATE_TRANSLATIONS)
        QT5_CREATE_TRANSLATION(cockatrice_QM ${translate_SRCS} ${cockatrice_TS})
    ELSE()
        QT5_ADD_TRANSLATION(cockatrice_QM ${cockatrice_TS})
    ENDIF()

    QT5_ADD_RESOURCES(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

# Declare path variables
set(ICONDIR share/icons CACHE STRING "icon dir")
set(DESKTOPDIR share/applications CACHE STRING "desktop file destination")

# Include directories
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Build cockatrice binary and link it
ADD_EXECUTABLE(cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_SOURCES} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC} ${cockatrice_MOC_SRCS})

if(MSVC)
    TARGET_LINK_LIBRARIES(cockatrice cockatrice_common Qt5::WinMain)
else()
    TARGET_LINK_LIBRARIES(cockatrice cockatrice_common)
endif()
qt5_use_modules(cockatrice ${COCKATRICE_LIBS})

if(UNIX)
    if(APPLE)
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
        set_target_properties(cockatrice PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

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

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir cockatrice.app/Contents/Plugins)
    set(qtconf_dest_dir cockatrice.app/Contents/Resources)
    get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE)

    # qt5 plugins: audio, iconengines, imageformats, platforms, printsupport
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(audio|iconengines|imageformats|platforms|printsupport)/.*\\.dylib"
        REGEX ".*_debug\\.dylib" EXCLUDE)

    install(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/cockatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()

if(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    # qt5 plugins: audio, iconengines, imageformats, platforms, printsupport
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(audio|iconengines|imageformats|platforms|printsupport)/.*[^d]\\.dll")

    install(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/cockatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)

    if(WIN32SSLRUNTIME_FOUND)
        install(FILES ${WIN32SSLRUNTIME_LIBRARIES} DESTINATION ./)
    endif()
endif()
#Compile a portable version, default off
option(PORTABLE "portable build" OFF)
IF(PORTABLE)
add_definitions(-DPORTABLE_BUILD)
endif()