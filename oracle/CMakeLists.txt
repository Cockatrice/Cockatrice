# CMakeLists for oracle directory
#
# provides the oracle binary

PROJECT(Oracle)

# paths
set(DESKTOPDIR share/applications CACHE STRING "path to .desktop files")

SET(oracle_SOURCES
    src/main.cpp
    src/oraclewizard.cpp
    src/oracleimporter.cpp
    ../cockatrice/src/carddatabase.cpp
    ../cockatrice/src/pictureloader.cpp
    ../cockatrice/src/carddbparser/cockatricexml3.cpp
    ../cockatrice/src/settingscache.cpp
    ../cockatrice/src/shortcutssettings.cpp
    ../cockatrice/src/settings/carddatabasesettings.cpp
    ../cockatrice/src/settings/serverssettings.cpp
    ../cockatrice/src/settings/settingsmanager.cpp
    ../cockatrice/src/settings/messagesettings.cpp
    ../cockatrice/src/settings/gamefilterssettings.cpp
    ../cockatrice/src/settings/layoutssettings.cpp
    ../cockatrice/src/thememanager.cpp
    ../cockatrice/src/qt-json/json.cpp
    ../cockatrice/src/releasechannel.cpp
     ${VERSION_STRING_CPP}
)

set(oracle_RESOURCES oracle.qrc)

IF(UPDATE_TRANSLATIONS)
    FILE(GLOB_RECURSE translate_oracle_SRCS src/*.cpp src/*.h ../cockatrice/src/settingscache.cpp)
    SET(translate_SRCS ${translate_oracle_SRCS})
    SET(oracle_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/oracle_en.ts")
ELSE()
    FILE(GLOB oracle_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
ENDIF(UPDATE_TRANSLATIONS)

if(WIN32)
    set(oracle_SOURCES ${oracle_SOURCES} oracle.rc)
endif(WIN32)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(oracle_SOURCES ${oracle_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

# Qt5
find_package(Qt5 COMPONENTS Concurrent Network Svg Widgets REQUIRED)
set(ORACLE_QT_MODULES Qt5::Concurrent Qt5::Network Qt5::Svg Qt5::Widgets)

# Qt5LinguistTools
find_package(Qt5LinguistTools)
if(Qt5LinguistTools_FOUND)
    list(APPEND ORACLE_LIBS Qt5::LinguistTools)

    if(NOT Qt5_LRELEASE_EXECUTABLE)
        MESSAGE(WARNING "Qt's lrelease not found.")
    endif()

    if(UPDATE_TRANSLATIONS)
        if(NOT Qt5_LUPDATE_EXECUTABLE)
            MESSAGE(WARNING "Qt's lupdate not found.")
        endif()
        QT5_CREATE_TRANSLATION(oracle_QM ${translate_SRCS} ${oracle_TS})
    else()
        QT5_ADD_TRANSLATION(oracle_QM ${oracle_TS})
    endif()
endif()

QT5_ADD_RESOURCES(oracle_RESOURCES_RCC ${oracle_RESOURCES})

INCLUDE_DIRECTORIES(../cockatrice/src)

# Libz is required to support zipped files
FIND_PACKAGE(ZLIB)
IF(ZLIB_FOUND)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
    ADD_DEFINITIONS("-DHAS_ZLIB")

    set(oracle_SOURCES ${oracle_SOURCES}
        src/zip/unzip.cpp
        src/zip/zipglobal.cpp
    )

ELSE()
    MESSAGE(STATUS "Oracle: zlib not found; ZIP support disabled")
ENDIF()

# Build oracle binary and link it
ADD_EXECUTABLE(oracle WIN32 MACOSX_BUNDLE ${oracle_SOURCES} ${oracle_QM} ${oracle_RESOURCES_RCC} ${oracle_MOC_SRCS})

IF(ZLIB_FOUND)
    TARGET_LINK_LIBRARIES(oracle ${ORACLE_QT_MODULES} ${ZLIB_LIBRARIES})
ELSE()
    TARGET_LINK_LIBRARIES(oracle ${ORACLE_QT_MODULES})
ENDIF()

if(UNIX)
    if(APPLE)
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
        set_target_properties(oracle PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

        INSTALL(TARGETS oracle BUNDLE DESTINATION ./)
        INSTALL(FILES ${oracle_QM} DESTINATION ./oracle.app/Contents/Resources/translations)
    else()
        # Assume linux
        INSTALL(TARGETS oracle RUNTIME DESTINATION bin/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
        INSTALL(FILES ${oracle_QM} DESTINATION share/oracle/translations)
    endif()
elseif(WIN32)
    INSTALL(TARGETS oracle RUNTIME DESTINATION ./)
    INSTALL(FILES ${oracle_QM} DESTINATION ./translations)
endif()

IF (NOT WIN32 AND NOT APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oracle.desktop DESTINATION ${DESKTOPDIR})
ENDIF (NOT WIN32 AND NOT APPLE)

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir oracle.app/Contents/Plugins)
    set(qtconf_dest_dir oracle.app/Contents/Resources)
    get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE)

    # qt5 plugins: iconengines, platforms
    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING
        PATTERN "*.dSYM" EXCLUDE
        PATTERN "*_debug.dylib" EXCLUDE
        PATTERN "iconengines/*.dylib"
        PATTERN "platforms/*.dylib"
        PATTERN "styles/*.dylib"
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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()

IF(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)
    list(APPEND libSearchDirs ${QT_LIBRARY_DIR})
    IF(ZLIB_FOUND)
        # look for dll in the bin/ directory (gnuwin32 package)
        get_filename_component(ZLIB_DLL_DIR "${ZLIB_INCLUDE_DIRS}/../bin/" REALPATH)
        list(APPEND libSearchDirs ${ZLIB_DLL_DIR})
        # look for dll in the lib/ directory (nuget package)
        get_filename_component(ZLIB_DLL_DIR "${ZLIB_LIBRARY}" DIRECTORY)
        list(APPEND libSearchDirs ${ZLIB_DLL_DIR})
    ENDIF()

    # qt5 plugins: iconengines, platforms

    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(iconengines|platforms)/.*[^d]\\.dll")

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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Oracle.exe\" \"\${QTPLUGINS}\" \"${libSearchDirs}\")
        " COMPONENT Runtime)
endif()
