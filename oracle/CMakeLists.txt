# CMakeLists for oracle directory
#
# provides the oracle binary

project(Oracle VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# paths
set(DESKTOPDIR
    share/applications
    CACHE STRING "path to .desktop files"
)

set(oracle_SOURCES
    src/main.cpp
    src/oraclewizard.cpp
    src/oracleimporter.cpp
    src/pagetemplates.cpp
    src/qt-json/json.cpp
    ../cockatrice/src/carddatabase.cpp
    ../cockatrice/src/pictureloader.cpp
    ../cockatrice/src/carddbparser/carddatabaseparser.cpp
    ../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../cockatrice/src/carddbparser/cockatricexml4.cpp
    ../cockatrice/src/settingscache.cpp
    ../cockatrice/src/shortcutssettings.cpp
    ../cockatrice/src/settings/carddatabasesettings.cpp
    ../cockatrice/src/settings/serverssettings.cpp
    ../cockatrice/src/settings/settingsmanager.cpp
    ../cockatrice/src/settings/messagesettings.cpp
    ../cockatrice/src/settings/gamefilterssettings.cpp
    ../cockatrice/src/settings/layoutssettings.cpp
    ../cockatrice/src/settings/downloadsettings.cpp
    ../cockatrice/src/thememanager.cpp
    ../cockatrice/src/releasechannel.cpp
    ${VERSION_STRING_CPP}
)

set(oracle_RESOURCES oracle.qrc)

if(UPDATE_TRANSLATIONS)
  file(GLOB_RECURSE translate_oracle_SRCS src/*.cpp src/*.h ../cockatrice/src/settingscache.cpp)
  set(translate_SRCS ${translate_oracle_SRCS})
  set(oracle_TS "${CMAKE_CURRENT_SOURCE_DIR}/oracle_en@source.ts")
else()
  file(GLOB oracle_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
endif(UPDATE_TRANSLATIONS)

if(WIN32)
  set(oracle_SOURCES ${oracle_SOURCES} oracle.rc)
endif(WIN32)

if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set(oracle_SOURCES ${oracle_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
endif(APPLE)

if(Qt6_FOUND)
  qt6_add_resources(oracle_RESOURCES_RCC ${oracle_RESOURCES})
elseif(Qt5_FOUND)
  qt5_add_resources(oracle_RESOURCES_RCC ${oracle_RESOURCES})
endif()

include_directories(../cockatrice/src)

# Libz is required to support zipped files
find_package(ZLIB)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
  add_definitions("-DHAS_ZLIB")

  set(oracle_SOURCES ${oracle_SOURCES} src/zip/unzip.cpp src/zip/zipglobal.cpp)
else()
  message(STATUS "Oracle: zlib not found; ZIP support disabled")
endif()

# LibLZMA is required to support xz files
find_package(LibLZMA)
if(LIBLZMA_FOUND)
  include_directories(${LIBLZMA_INCLUDE_DIRS})
  add_definitions("-DHAS_LZMA")

  set(oracle_SOURCES ${oracle_SOURCES} src/lzma/decompress.cpp)
else()
  message(STATUS "Oracle: LibLZMA not found; xz support disabled")
endif()

set(ORACLE_MAC_QM_INSTALL_DIR "oracle.app/Contents/Resources/translations")
set(ORACLE_UNIX_QM_INSTALL_DIR "share/oracle/translations")
set(ORACLE_WIN32_QM_INSTALL_DIR "translations")

if(Qt6_FOUND)
  # Qt6 Translations are linked after the executable is created in manual mode
  qt6_add_executable(
    oracle
    WIN32
    MACOSX_BUNDLE
    ${oracle_SOURCES}
    ${oracle_RESOURCES_RCC}
    ${oracle_MOC_SRCS}
    MANUAL_FINALIZATION
  )
elseif(Qt5_FOUND)
  # Qt5 Translations need to be linked at executable creation time
  if(Qt5LinguistTools_FOUND)
    if(UPDATE_TRANSLATIONS)
      qt5_create_translation(oracle_QM ${translate_SRCS} ${oracle_TS})
    else()
      qt5_add_translation(oracle_QM ${oracle_TS})
    endif()
  endif()
  add_executable(oracle WIN32 MACOSX_BUNDLE ${oracle_MOC_SRCS} ${oracle_QM} ${oracle_RESOURCES_RCC} ${oracle_SOURCES})
  if(UNIX)
    if(APPLE)
      install(FILES ${oracle_QM} DESTINATION ${ORACLE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${oracle_QM} DESTINATION ${ORACLE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${oracle_QM} DESTINATION ${ORACLE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

target_link_libraries(oracle PUBLIC ${ORACLE_QT_MODULES})

if(ZLIB_FOUND)
  target_link_libraries(oracle PUBLIC ${ZLIB_LIBRARIES})
endif()

if(LIBLZMA_FOUND)
  target_link_libraries(oracle PUBLIC ${LIBLZMA_LIBRARIES})
endif()

if(UNIX)
  if(APPLE)
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set_target_properties(oracle PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

    install(TARGETS oracle BUNDLE DESTINATION ./)
  else()
    # Assume linux
    install(TARGETS oracle RUNTIME DESTINATION bin/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
  endif()
elseif(WIN32)
  install(TARGETS oracle RUNTIME DESTINATION ./)
endif()

if(NOT WIN32 AND NOT APPLE)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oracle.desktop DESTINATION ${DESKTOPDIR})
endif(NOT WIN32 AND NOT APPLE)

if(APPLE)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir oracle.app/Contents/Plugins)
  set(qtconf_dest_dir oracle.app/Contents/Resources)

  # Qt plugins: iconengines, platforms, styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "*.dSYM" EXCLUDE
    PATTERN "*_debug.dylib" EXCLUDE
    PATTERN "iconengines/*.dylib"
    PATTERN "platforms/*.dylib"
    PATTERN "styles/*.dylib"
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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        "
    COMPONENT Runtime
  )
endif()

if(WIN32)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir Plugins)
  set(qtconf_dest_dir .)
  list(APPEND libSearchDirs ${QT_LIBRARY_DIR})

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}/"
    DESTINATION ./
    FILES_MATCHING
    PATTERN "*.dll"
  )

  # Qt plugins: iconengines, platforms, styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "iconengines/qsvgicon.dll"
    PATTERN "platforms/qdirect2d.dll"
    PATTERN "platforms/qminimal.dll"
    PATTERN "platforms/qoffscreen.dll"
    PATTERN "platforms/qwindows.dll"
    PATTERN "styles/qcertonlybackend.dll"
    PATTERN "styles/qopensslbackend.dll"
    PATTERN "styles/qschannelbackend.dll"
    PATTERN "styles/qwindowsvistastyle.dll"
    PATTERN "tls/qcertonlybackend.dll"
    PATTERN "tls/qopensslbackend.dll"
    PATTERN "tls/qschannelbackend.dll"
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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Oracle.exe\" \"\${QTPLUGINS}\" \"${libSearchDirs}\")
        "
    COMPONENT Runtime
  )
endif()

if(Qt6LinguistTools_FOUND)
  #Qt6 Translations happen after the executable is built up
  if(UPDATE_TRANSLATIONS)
    qt6_add_translations(
      oracle
      TS_FILES
      ${oracle_TS}
      SOURCES
      ${translate_SRCS}
      QM_FILES_OUTPUT_VARIABLE
      oracle_QM
    )
  else()
    qt6_add_translations(oracle TS_FILES ${oracle_TS} QM_FILES_OUTPUT_VARIABLE oracle_QM)
  endif()

  if(UNIX)
    if(APPLE)
      install(FILES ${oracle_QM} DESTINATION ${ORACLE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${oracle_QM} DESTINATION ${ORACLE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${oracle_QM} DESTINATION ${ORACLE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

if(Qt6_FOUND)
  qt6_finalize_target(oracle)
endif()
