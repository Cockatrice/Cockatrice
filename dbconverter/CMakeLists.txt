# CMakeLists for dbconverter directory

PROJECT(Dbconverter VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

SET(dbconverter_SOURCES
    src/main.cpp
    src/mocks.cpp
    ../cockatrice/src/carddatabase.cpp
    ../cockatrice/src/carddbparser/carddatabaseparser.cpp
    ../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../cockatrice/src/carddbparser/cockatricexml4.cpp
    ../cockatrice/src/settings/settingsmanager.cpp
    ${VERSION_STRING_CPP}
)


SET(QT_DONT_USE_QTGUI TRUE)

IF(Qt6_FOUND)
    Qt6_WRAP_CPP(dbconverter_SOURCES
        ../cockatrice/src/settingscache.h
        ../cockatrice/src/settings/carddatabasesettings.h
    )
ELSEIF(Qt5_FOUND)
    Qt5_WRAP_CPP(dbconverter_SOURCES
            ../cockatrice/src/settingscache.h
            ../cockatrice/src/settings/carddatabasesettings.h
            )
ENDIF()

# Build servatrice binary and link it
ADD_EXECUTABLE(dbconverter MACOSX_BUNDLE ${dbconverter_SOURCES} ${dbconverter_MOC_SRCS})

TARGET_LINK_LIBRARIES(dbconverter ${DB_CONVERTER_QT_MODULES})

# install rules
if(UNIX)
    if(APPLE)
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

        INSTALL(TARGETS dbconverter BUNDLE DESTINATION ./)
    else()
        # Assume linux
        INSTALL(TARGETS dbconverter RUNTIME DESTINATION bin/)
    endif()
elseif(WIN32)
    INSTALL(TARGETS dbconverter RUNTIME DESTINATION ./)
endif()

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir dbconverter.app/Contents/Plugins)
    set(qtconf_dest_dir dbconverter.app/Contents/Resources)

    # Qt plugins: platforms
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING
        PATTERN "*.dSYM" EXCLUDE
        PATTERN "*_debug.dylib" EXCLUDE
        PATTERN "platforms/*.dylib"
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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/dbconverter.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR};${MYSQLCLIENT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()

if(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    install(DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}/" DESTINATION ./ FILES_MATCHING PATTERN "*.dll")

    # Qt plugins: platforms
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING PATTERN
            "platforms/qdirect2d.dll"
            "platforms/qminimal.dll"
            "platforms/qoffscreen.dll"
            "platforms/qwindows.dll")

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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/dbconverter.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()
