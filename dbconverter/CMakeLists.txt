# CMakeLists for dbconverter directory

project(Dbconverter VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(dbconverter_SOURCES
    src/main.cpp
    src/mocks.cpp
    ../cockatrice/src/game/cards/card_database.cpp
    ../cockatrice/src/game/cards/card_database_parser/card_database_parser.cpp
    ../cockatrice/src/game/cards/card_database_parser/cockatrice_xml_3.cpp
    ../cockatrice/src/game/cards/card_database_parser/cockatrice_xml_4.cpp
    ../cockatrice/src/game/cards/card_info.cpp
    ../cockatrice/src/game/cards/exact_card.cpp
    ../cockatrice/src/settings/settings_manager.cpp
    ${VERSION_STRING_CPP}
)

set(QT_DONT_USE_QTGUI TRUE)

if(Qt6_FOUND)
  qt6_wrap_cpp(
    dbconverter_SOURCES ../cockatrice/src/settings/cache_settings.h ../cockatrice/src/settings/card_database_settings.h
  )
elseif(Qt5_FOUND)
  qt5_wrap_cpp(
    dbconverter_SOURCES ../cockatrice/src/settings/cache_settings.h ../cockatrice/src/settings/card_database_settings.h
  )
endif()

# Include directories
include_directories(../common) # Required due to card_ref.h

# Build servatrice binary and link it
add_executable(dbconverter MACOSX_BUNDLE ${dbconverter_MOC_SRCS} ${dbconverter_SOURCES})

target_link_libraries(dbconverter ${DB_CONVERTER_QT_MODULES})

# install rules
if(UNIX)
  if(APPLE)
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

    install(TARGETS dbconverter BUNDLE DESTINATION ./)
  else()
    # Assume linux
    install(TARGETS dbconverter RUNTIME DESTINATION bin/)
  endif()
elseif(WIN32)
  install(TARGETS dbconverter RUNTIME DESTINATION ./)
endif()

if(APPLE)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir dbconverter.app/Contents/Plugins)
  set(qtconf_dest_dir dbconverter.app/Contents/Resources)

  # Qt plugins: platforms
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "*.dSYM" EXCLUDE
    PATTERN "*_debug.dylib" EXCLUDE
    PATTERN "platforms/*.dylib"
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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/dbconverter.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
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

  # Qt plugins: platforms
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "platforms/qdirect2d.dll"
    PATTERN "platforms/qminimal.dll"
    PATTERN "platforms/qoffscreen.dll"
    PATTERN "platforms/qwindows.dll"
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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/dbconverter.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    "
    COMPONENT Runtime
  )
endif()
