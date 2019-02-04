# CMakeLists for servatrice directory
#
# provides the servatrice binary

PROJECT(Servatrice VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

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

# Under FreeBSD we need libexecinfo to use backtrace_symbols_fd()
if(CMAKE_HOST_SYSTEM MATCHES "FreeBSD")
  find_package(Libexecinfo REQUIRED)
  set(SYSTEM_LIBRARIES ${EXECINFO_LIBRARY} ${SYSTEM_LIBRARIES})
endif()


if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(servatrice_SOURCES ${servatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

# Qt5
find_package(Qt5 COMPONENTS Network Sql WebSockets REQUIRED)
set(SERVATRICE_QT_MODULES Qt5::Core Qt5::Network Qt5::Sql Qt5::WebSockets)

QT5_ADD_RESOURCES(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})
SET(QT_DONT_USE_QTGUI TRUE)

# Mysql connector
if(UNIX)
    if(APPLE)
        SET(MYSQLCLIENT_DEFAULT_PATHS "/usr/local/lib" "/opt/local/lib/mysql55/mysql/" "/opt/local/lib/mysql56/mysql/")
    else()
        SET(MYSQLCLIENT_DEFAULT_PATHS "/usr/lib64" "/usr/local/lib64" "/usr/lib" "/usr/local/lib")
    endif()
elseif(WIN32)
    SET(MYSQLCLIENT_DEFAULT_PATHS "C:\\Program Files\\MySQL\\MySQL Server 5.7\\lib" "C:\\Program Files (x86)\\MySQL\\MySQL Server 5.7\\lib")
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
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${CMAKE_THREAD_LIBS_INIT} ${SERVATRICE_QT_MODULES} Qt5::WinMain)
elseif(CMAKE_HOST_SYSTEM MATCHES "FreeBSD")
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${CMAKE_THREAD_LIBS_INIT} ${SERVATRICE_QT_MODULES} ${LIBEXECINFO_LIBRARY})
else()
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${CMAKE_THREAD_LIBS_INIT} ${SERVATRICE_QT_MODULES})
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
    get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE)

    # qt5 plugins: platforms, sqldrivers/mysql
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING
        PATTERN "*.dSYM" EXCLUDE
        PATTERN "*_debug.dylib" EXCLUDE
        PATTERN "platforms/*.dylib"
        PATTERN "sqldrivers/libqsqlmysql*.dylib"
    )

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

    install(DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}/" DESTINATION ./ FILES_MATCHING PATTERN "*.dll")

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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Servatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()
