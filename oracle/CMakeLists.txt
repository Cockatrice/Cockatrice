# CMakeLists for oracle directory
#
# provides the oracle binary

PROJECT(oracle)

# paths
set(DESKTOPDIR share/applications CACHE STRING "path to .desktop files")

SET(oracle_SOURCES
    src/main.cpp
    src/oracleimporter.cpp
    src/window_main.cpp
    ../cockatrice/src/carddatabase.cpp
    ../cockatrice/src/settingscache.cpp
 )

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
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND ORACLE_LIBS Widgets)

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
endif()

INCLUDE_DIRECTORIES(../cockatrice/src)

# Build oracle binary and link it
ADD_EXECUTABLE(oracle WIN32 MACOSX_BUNDLE ${oracle_SOURCES} ${oracle_MOC_SRCS})

if(Qt4_FOUND)
    TARGET_LINK_LIBRARIES(oracle ${ORACLE_LIBS})
endif()
if(Qt5Widgets_FOUND)
    qt5_use_modules(oracle ${ORACLE_LIBS})
endif()

if(MSVC)
	set_target_properties(oracle PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(MSVC)

if(UNIX)
    if(APPLE)
        INSTALL(TARGETS oracle BUNDLE DESTINATION ./)
    else()
        # Assume linux
        INSTALL(TARGETS oracle RUNTIME DESTINATION bin/)
    endif()
elseif(WIN32)
    INSTALL(TARGETS oracle RUNTIME DESTINATION ./)
endif()

IF (NOT WIN32 AND NOT APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oracle.desktop DESTINATION ${DESKTOPDIR})
ENDIF (NOT WIN32 AND NOT APPLE)

if(APPLE)
    set(plugin_dest_dir ./oracle.app/Contents/Plugins)
    set(qtconf_dest_dir ./oracle.app/Contents/Resources)

    # note: no codecs in qt5
    # note: phonon_backend => mediaservice
    # note: needs platform on osx

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/.*_debug\\.dylib")
    else()
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/[^_]*\\.dylib")
    endif()

    install(CODE "
        file(WRITE \"${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins\")
        " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"${plugin_dest_dir}/*.dylib\")
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()
