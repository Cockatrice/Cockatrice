# CMakeLists for oracle directory
#
# provides the oracle binary

PROJECT(oracle)

# paths
set(DESKTOPDIR share/applications CACHE STRING "path to .desktop files")

SET(oracle_SOURCES
    src/main.cpp
    src/oraclewizard.cpp
    src/oracleimporter.cpp
    ../cockatrice/src/carddatabase.cpp
    ../cockatrice/src/settingscache.cpp
    ../cockatrice/src/qt-json/json.cpp
 )

set(oracle_RESOURCES oracle.qrc)

if(WIN32)
    set(oracle_SOURCES ${oracle_SOURCES} oracle.rc)
endif(WIN32)


if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(oracle_SOURCES ${oracle_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
ENDIF(APPLE)

set(ORACLE_LIBS)

# Qt4 stuff
if(Qt4_FOUND)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTXML TRUE)
    SET(QT_USE_QTSVG TRUE)

    # Include directories
    INCLUDE(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
    LIST(APPEND ORACLE_LIBS ${QT_QTMAIN_LIBRARY})
    LIST(APPEND ORACLE_LIBS ${QT_LIBRARIES})

    # Let cmake chew Qt4's translations and resource files
    # Note: header files are MOC-ed automatically by cmake
    QT4_ADD_RESOURCES(oracle_RESOURCES_RCC ${oracle_RESOURCES})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND ORACLE_LIBS Widgets)

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
        list(APPEND ORACLE_LIBS Network)
    endif()

    # QtXml
    find_package(Qt5Xml)
    if(Qt5Xml_FOUND)
        include_directories(${Qt5Xml_INCLUDE_DIRS})
        list(APPEND ORACLE_LIBS Xml)
    endif()

    # QtSvg
    find_package(Qt5Svg)
    if(Qt5Svg_FOUND)
        include_directories(${Qt5Svg_INCLUDE_DIRS})
        list(APPEND ORACLE_LIBS Svg)
    endif()

    # Let cmake chew Qt5's translations and resource files
    # Note: header files are MOC-ed automatically by cmake
    QT5_ADD_RESOURCES(oracle_RESOURCES_RCC ${oracle_RESOURCES})

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

INCLUDE_DIRECTORIES(../cockatrice/src)

# Build oracle binary and link it
ADD_EXECUTABLE(oracle WIN32 MACOSX_BUNDLE ${oracle_SOURCES} ${oracle_RESOURCES_RCC} ${oracle_MOC_SRCS})

if(Qt4_FOUND)
    if(MSVC) 
        set(QT_USE_QTMAIN true) 
    endif() 
    TARGET_LINK_LIBRARIES(oracle ${ORACLE_LIBS})
endif()
if(Qt5Widgets_FOUND)
    if(MSVC)
        TARGET_LINK_LIBRARIES(oracle Qt5::WinMain)
    endif()
    qt5_use_modules(oracle ${ORACLE_LIBS})
endif()

if(UNIX)
    if(APPLE)
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

        INSTALL(TARGETS oracle BUNDLE DESTINATION ./)
    else()
        # Assume linux
        INSTALL(TARGETS oracle RUNTIME DESTINATION bin/)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/oracle.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
    endif()
elseif(WIN32)
    INSTALL(TARGETS oracle RUNTIME DESTINATION ./)
endif()

IF (NOT WIN32 AND NOT APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oracle.desktop DESTINATION ${DESKTOPDIR})
ENDIF (NOT WIN32 AND NOT APPLE)

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir oracle.app/Contents/Plugins)
    set(qtconf_dest_dir oracle.app/Contents/Resources)

    # qt4: codecs, iconengines, imageformats
    # qt5: iconengines, platforms

    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(codecs|iconengines|platforms)/.*\\.dylib"
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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()

IF(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    # qt4: codecs, iconengines, imageformats
    # qt5: iconengines, imageformats, platforms

    install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(codecs|iconengines|platforms)/.*[^d]\\.dll")

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
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif() 