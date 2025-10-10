# CMakeLists for servatrice directory
#
# provides the servatrice binary

project(Servatrice VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(servatrice_SOURCES
    src/email_parser.cpp
    src/main.cpp
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
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set(servatrice_SOURCES ${servatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
endif(APPLE)

if(Qt6_FOUND)
  qt6_add_resources(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})
elseif(Qt5_FOUND)
  qt5_add_resources(servatrice_RESOURCES_RCC ${servatrice_RESOURCES})
endif()

set(QT_DONT_USE_QTGUI TRUE)

# Mysql connector
if(UNIX)
  if(APPLE)
    set(MYSQLCLIENT_DEFAULT_PATHS "/usr/local/lib" "/opt/local/lib/mysql55/mysql/" "/opt/local/lib/mysql56/mysql/")
  else()
    set(MYSQLCLIENT_DEFAULT_PATHS "/usr/lib64" "/usr/local/lib64" "/usr/lib" "/usr/local/lib")
  endif()
elseif(WIN32)
  set(MYSQLCLIENT_DEFAULT_PATHS "C:\\Program Files\\MySQL\\MySQL Server 5.7\\lib"
                                "C:\\Program Files (x86)\\MySQL\\MySQL Server 5.7\\lib"
  )
endif()

find_library(
  MYSQL_CLIENT_LIBRARIES
  NAMES mysqlclient
  PATHS ${MYSQLCLIENT_DEFAULT_PATHS}
  PATH_SUFFIXES mysql mariadb
)
if(${MYSQL_CLIENT_LIBRARIES} MATCHES "NOTFOUND")
  set(MYSQLCLIENT_FOUND
      FALSE
      CACHE INTERNAL ""
  )
  message(STATUS "MySQL connector NOT FOUND: Servatrice won't be able to connect to a MySQL server")
  unset(MYSQL_CLIENT_LIBRARIES)
else()
  set(MYSQLCLIENT_FOUND
      TRUE
      CACHE INTERNAL ""
  )
  get_filename_component(MYSQLCLIENT_LIBRARY_DIR ${MYSQL_CLIENT_LIBRARIES} PATH)
  message(STATUS "Found MySQL connector at: ${MYSQL_CLIENT_LIBRARIES}")
endif()

# Declare path variables
set(ICONDIR
    share/icons
    CACHE STRING "icon dir"
)
set(DESKTOPDIR
    share/applications
    CACHE STRING "desktop file destination"
)

# Build servatrice binary and link it
add_executable(servatrice MACOSX_BUNDLE ${servatrice_MOC_SRCS} ${servatrice_RESOURCES_RCC} ${servatrice_SOURCES})

if(CMAKE_HOST_SYSTEM MATCHES "FreeBSD")
  target_link_libraries(
    servatrice libcockatrice_deck_list libcockatrice_network_server_remote Threads::Threads ${SERVATRICE_QT_MODULES}
    ${LIBEXECINFO_LIBRARY}
  )
else()
  target_link_libraries(
    servatrice libcockatrice_deck_list libcockatrice_network_server_remote Threads::Threads ${SERVATRICE_QT_MODULES}
  )
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

    install(TARGETS servatrice BUNDLE DESTINATION ./)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION ./servatrice.app/Contents/Resources/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION ./servatrice.app/Contents/Resources/)
  else()
    # Assume linux
    install(TARGETS servatrice RUNTIME DESTINATION bin/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION share/servatrice/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION share/servatrice/)

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/servatrice.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/servatrice.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.desktop DESTINATION ${DESKTOPDIR})
  endif()
elseif(WIN32)
  install(TARGETS servatrice RUNTIME DESTINATION ./)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.ini.example DESTINATION ./)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/servatrice.sql DESTINATION ./)
endif()

if(APPLE)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir servatrice.app/Contents/Plugins)
  set(qtconf_dest_dir servatrice.app/Contents/Resources)

  # Qt plugins: platforms, sqldrivers/mysql, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "*.dSYM" EXCLUDE
    PATTERN "*_debug.dylib" EXCLUDE
    PATTERN "platforms/*.dylib"
    PATTERN "sqldrivers/libqsqlmysql*.dylib"
    PATTERN "tls/*.dylib"
  )

  install(
    CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
        \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    "
    COMPONENT Runtime
  )
endif()

if(WIN32)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir Plugins)
  set(qtconf_dest_dir .)

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}/"
    DESTINATION ./
    FILES_MATCHING
    PATTERN "*.dll"
  )

  # Qt plugins: platforms, sqldrivers, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "platforms/qdirect2d.dll"
    PATTERN "platforms/qminimal.dll"
    PATTERN "platforms/qoffscreen.dll"
    PATTERN "platforms/qwindows.dll"
    PATTERN "tls/qcertonlybackend.dll"
    PATTERN "tls/qopensslbackend.dll"
    PATTERN "tls/qschannelbackend.dll"
    PATTERN "sqldrivers/qsqlite.dll"
    PATTERN "sqldrivers/qsqlodbc.dll"
    PATTERN "sqldrivers/qsqlpsql.dll"
  )

  install(
    CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
        \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Servatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    "
    COMPONENT Runtime
  )
endif()
