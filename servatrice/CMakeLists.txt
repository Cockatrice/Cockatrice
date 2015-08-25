# CMakeLists for servatrice directory
#
# provides the servatrice binary

PROJECT(servatrice)

SET(servatrice_SOURCES
    src/main.cpp
    src/passwordhasher.cpp
    src/servatrice.cpp
    src/servatrice_connection_pool.cpp
    src/servatrice_database_interface.cpp
    src/server_logger.cpp
    src/serversocketinterface.cpp
    src/settingscache.cpp
    src/isl_interface.cpp
    src/signalhandler.cpp
    ${VERSION_STRING_CPP}
    src/smtpclient.cpp
    src/smtp/qxthmac.cpp
    src/smtp/qxtmailattachment.cpp
    src/smtp/qxtmailmessage.cpp
    src/smtp/qxtsmtp.cpp
)

set(servatrice_RESOURCES servatrice.qrc)

if(WIN32)
    set(servatrice_SOURCES ${servatrice_SOURCES} servatrice.rc)
endif(WIN32)


if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(servatrice_SOURCES ${servatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

set(SERVATRICE_LIBS)

# Qt4 stuff
if(Qt4_FOUND)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTSQL TRUE)

    # Include directories
    INCLUDE(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
    LIST(APPEND SERVATRICE_LIBS ${QT_LIBRARIES})

    # Libgcrypt is required only with Qt4 to support SHA512 hashing
    FIND_PACKAGE(Libgcrypt REQUIRED)
    INCLUDE_DIRECTORIES(${LIBGCRYPT_INCLUDE_DIR})
    QT4_ADD_RESOURCES(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND SERVATRICE_LIBS Widgets)

    # QtNetwork
    find_package(Qt5Network)
    if(Qt5Network_FOUND)
        include_directories(${Qt5Network_INCLUDE_DIRS})
        list(APPEND SERVATRICE_LIBS Network)
    endif()

    # QtSql
    find_package(Qt5Sql)
    if(Qt5Sql_FOUND)
        include_directories(${Qt5Sql_INCLUDE_DIRS})
        list(APPEND SERVATRICE_LIBS Sql)
    endif()

    QT5_ADD_RESOURCES(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

SET(QT_DONT_USE_QTGUI TRUE)

# Declare path variables
set(ICONDIR share/icons CACHE STRING "icon dir")
set(DESKTOPDIR share/applications CACHE STRING "desktop file destination")

# Include directories
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Build servatrice binary and link it
ADD_EXECUTABLE(servatrice MACOSX_BUNDLE ${servatrice_SOURCES} ${servatrice_RESOURCES_RCC} ${servatrice_MOC_SRCS})
set_property(TARGET servatrice PROPERTY CXX_STANDARD 11)
set_property(TARGET servatrice PROPERTY CXX_STANDARD_REQUIRED ON)

if(Qt4_FOUND)
    if(MSVC) 
        set(QT_USE_QTMAIN true) 
    endif() 
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${SERVATRICE_LIBS} ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
endif()
if(Qt5Widgets_FOUND)
    if(MSVC)
        TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} Qt5::WinMain)
    else()
        TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    endif()
    qt5_use_modules(servatrice ${SERVATRICE_LIBS})
endif()

# install rules
if(UNIX)
    if(APPLE)
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

        INSTALL(TARGETS servatrice BUNDLE DESTINATION ./)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION ./servatrice.app/Contents/Resources/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION ./servatrice.app/Contents/Resources/)
    else()
        # Assume linux
        INSTALL(TARGETS servatrice RUNTIME DESTINATION bin/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION share/servatrice/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION share/servatrice/)

        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/servatrice.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/servatrice.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.desktop DESTINATION ${DESKTOPDIR})
    endif()
elseif(WIN32)
    INSTALL(TARGETS servatrice RUNTIME DESTINATION ./)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION ./)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION ./)
endif()

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir servatrice.app/Contents/Plugins)
    set(qtconf_dest_dir servatrice.app/Contents/Resources)

    # qt4: codecs, sqldrivers
    # qt5: platforms, sqldrivers

    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(codecs|platforms|sqldrivers)/.*\\.dylib"
        REGEX ".*_debug\\.dylib" EXCLUDE)

    install(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations\")
    " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
        \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()

if(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    # qt4: codecs, sqldrivers
    # qt5: platforms, sqldrivers

    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(codecs|platforms|sqldrivers)/.*[^d]\\.dll")

    install(CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations\")
    " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
        \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()
#Compile a portable version, default off
option(PORTABLE "portable build" OFF)
IF(PORTABLE)
add_definitions(-DPORTABLE_BUILD)
endif()
