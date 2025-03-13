# CMakeLists for cockatrice directory
#
# provides the cockatrice binary

project(Cockatrice VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

file(GLOB_RECURSE cockatrice_CPP_FILES ${CMAKE_SOURCE_DIR}/cockatrice/src/*.cpp)

set(cockatrice_SOURCES
    ${cockatrice_CPP_FILES}
    ${VERSION_STRING_CPP}
)

add_subdirectory(sounds)
add_subdirectory(themes)
configure_file(
  ${CMAKE_SOURCE_DIR}/cockatrice/resources/config/qtlogging.ini ${CMAKE_BINARY_DIR}/cockatrice/qtlogging.ini COPYONLY
)

set(cockatrice_RESOURCES cockatrice.qrc)

if(UPDATE_TRANSLATIONS)
  file(GLOB_RECURSE translate_cockatrice_SRCS ${CMAKE_SOURCE_DIR}/cockatrice/src/*.cpp
       ${CMAKE_SOURCE_DIR}/cockatrice/src/*.h
  )
  file(GLOB_RECURSE translate_common_SRCS ${CMAKE_SOURCE_DIR}/common/*.cpp ${CMAKE_SOURCE_DIR}/common/*.h)
  set(translate_SRCS ${translate_cockatrice_SRCS} ${translate_common_SRCS})
  set(cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice_en@source.ts")
else()
  file(GLOB cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
endif(UPDATE_TRANSLATIONS)

if(WIN32)
  set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set(cockatrice_SOURCES ${cockatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
endif(APPLE)

if(Qt6_FOUND)
  qt6_add_resources(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})
elseif(Qt5_FOUND)
  qt5_add_resources(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})
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

# Include directories
include_directories(../common)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/common)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(COCKATRICE_MAC_QM_INSTALL_DIR "cockatrice.app/Contents/Resources/translations")
set(COCKATRICE_UNIX_QM_INSTALL_DIR "share/cockatrice/translations")
set(COCKATRICE_WIN32_QM_INSTALL_DIR "translations")

if(Qt6_FOUND)
  qt6_add_executable(
    cockatrice
    WIN32
    MACOSX_BUNDLE
    ${cockatrice_SOURCES}
    ${cockatrice_RESOURCES_RCC}
    ${cockatrice_MOC_SRCS}
    MANUAL_FINALIZATION
  )
elseif(Qt5_FOUND)
  # Qt5 Translations need to be linked at executable creation time
  if(Qt5LinguistTools_FOUND)
    if(UPDATE_TRANSLATIONS)
      qt5_create_translation(cockatrice_QM ${translate_SRCS} ${cockatrice_TS})
    else()
      qt5_add_translation(cockatrice_QM ${cockatrice_TS})
    endif()
  endif()
  add_executable(
    cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_MOC_SRCS} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC}
                                   ${cockatrice_SOURCES}
  )
  if(UNIX)
    if(APPLE)
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

if(Qt5_FOUND)
  target_link_libraries(cockatrice cockatrice_common ${COCKATRICE_QT_MODULES})
else()
  target_link_libraries(cockatrice PUBLIC cockatrice_common ${COCKATRICE_QT_MODULES})
endif()

if(UNIX)
  if(APPLE)
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

    set_target_properties(cockatrice PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

    install(TARGETS cockatrice BUNDLE DESTINATION ./)
  else()
    # Assume linux
    install(TARGETS cockatrice RUNTIME DESTINATION bin/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cockatrice.desktop DESTINATION ${DESKTOPDIR})
  endif()
elseif(WIN32)
  install(TARGETS cockatrice RUNTIME DESTINATION ./)
endif()

if(APPLE)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir cockatrice.app/Contents/Plugins)
  set(qtconf_dest_dir cockatrice.app/Contents/Resources)

  # Qt plugins: audio (Qt5), iconengines, imageformats, multimedia (Qt6), platforms, printsupport (Qt5), styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "*.dSYM" EXCLUDE
    PATTERN "*_debug.dylib" EXCLUDE
    PATTERN "audio/*.dylib"
    PATTERN "iconengines/*.dylib"
    PATTERN "imageformats/*.dylib"
    PATTERN "multimedia/*.dylib"
    PATTERN "platforms/*.dylib"
    PATTERN "printsupport/*.dylib"
    PATTERN "styles/*.dylib"
    PATTERN "tls/*.dylib"
  )

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/cockatrice/"
    DESTINATION ${qtconf_dest_dir}/
    FILES_MATCHING
    PATTERN "*.ini"
  )

  install(
    CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/cockatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
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

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/cockatrice/"
    DESTINATION ./
    FILES_MATCHING
    PATTERN "*.ini"
  )

  # Qt plugins: audio (Qt5), iconengines, imageformats, multimedia (Qt6) platforms, printsupport (Qt5), styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "audio/qtaudio_wasapi.dll"
    PATTERN "audio/qtaudio_windows.dll"
    PATTERN "iconengines/qsvgicon.dll"
    PATTERN "imageformats/*.dll"
    PATTERN "mediaservice/dsengine.dll"
    PATTERN "mediaservice/wmfengine.dll"
    PATTERN "multimedia/*.dll"
    PATTERN "platforms/qdirect2d.dll"
    PATTERN "platforms/qminimal.dll"
    PATTERN "platforms/qoffscreen.dll"
    PATTERN "platforms/qwindows.dll"
    PATTERN "printsupport/windowsprintersupport.dll"
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
Translations = Resources/translations
Data = Resources\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Cockatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        "
    COMPONENT Runtime
  )

  if(OPENSSL_FOUND)
    install(FILES ${OPENSSL_INCLUDE_DIRS} DESTINATION ./)
  endif()
endif()

if(Qt6_FOUND AND Qt6LinguistTools_FOUND)
  #Qt6 Translations happen after the executable is built up
  if(UPDATE_TRANSLATIONS)
    qt6_add_translations(
      cockatrice
      TS_FILES
      ${cockatrice_TS}
      SOURCES
      ${translate_SRCS}
      QM_FILES_OUTPUT_VARIABLE
      cockatrice_QM
    )
  else()
    qt6_add_translations(cockatrice TS_FILES ${cockatrice_TS} QM_FILES_OUTPUT_VARIABLE cockatrice_QM)
  endif()

  if(UNIX)
    if(APPLE)
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

if(Qt6_FOUND)
  qt6_finalize_target(cockatrice)
endif()
