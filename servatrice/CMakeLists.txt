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

    # QtWebsockets
    find_package(Qt5WebSockets)
    if(Qt5WebSockets_FOUND)
        include_directories(${Qt5WebSockets_INCLUDE_DIRS})
        list(APPEND SERVATRICE_LIBS WebSockets)
    endif()

    QT5_ADD_RESOURCES(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

SET(QT_DONT_USE_QTGUI TRUE)

# Mysql connector
if(UNIX)
    if(APPLE)
        SET(MYSQLCLIENT_DEFAULT_PATHS "/usr/local/lib" "/opt/local/lib/mysql55/mysql/" "/opt/local/lib/mysql56/mysql/")
    else()
        SET(MYSQLCLIENT_DEFAULT_PATHS "/usr/lib64" "/usr/local/lib64" "/usr/lib" "/usr/local/lib")
    endif()
elseif(WIN32)
    SET(MYSQLCLIENT_DEFAULT_PATHS "C:\\Program Files\\MySQL\\MySQL Server 5.5\\lib" "C:\\Program Files\\MySQL\\MySQL Server 5.6\\lib" "C:\\Program Files\\MySQL\\MySQL Server 5.7\\lib")
endif()

find_library(MYSQLCLIENT_LIBRARIES NAMES mysqlclient PATHS ${MYSQLCLIENT_DEFAULT_PATHS} PATH_SUFFIXES mysql mariadb)
if(${MYSQLCLIENT_LIBRARIES} MATCHES "NOTFOUND")
  set(MYSQLCLIENT_FOUND FALSE CACHE INTERNAL "")
  MESSAGE(STATUS "Mysql connector NOT FOUND: servatrice won't be able to connect to a mysql server")
  unset(MYSQLCLIENT_LIBRARIES)
else()
  set(MYSQLCLIENT_FOUND TRUE CACHE INTERNAL "")
  get_filename_component(MYSQLCLIENT_LIBRARY_DIR ${MYSQLCLIENT_LIBRARIES} PATH)
  MESSAGE(STATUS "Mysql connector found at: ${MYSQLCLIENT_LIBRARY_DIR}")
endif()

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

if(MSVC)
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${CMAKE_THREAD_LIBS_INIT} Qt5::WinMain)
else()
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${CMAKE_THREAD_LIBS_INIT})
endif()
qt5_use_modules(servatrice ${SERVATRICE_LIBS})

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
    get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE)

    # qt5 plugins: platforms, sqldrivers/mysql
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(platforms/.*|sqldrivers/libqsqlmysql)\\.dylib"
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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()

if(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    # qt5 plugins: platforms, sqldrivers/mysql
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(platforms/.*|sqldrivers/qsqlmysql)\\.dll"
        REGEX ".*d\\.dll" EXCLUDE)

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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()
#Compile a portable version, default off
option(PORTABLE "portable build" OFF)
IF(PORTABLE)
add_definitions(-DPORTABLE_BUILD)
endif()
