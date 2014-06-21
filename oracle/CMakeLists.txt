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

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)

# Include directories
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../cockatrice/src)

# Build oracle binary and link it
ADD_EXECUTABLE(oracle WIN32 MACOSX_BUNDLE ${oracle_SOURCES} ${oracle_MOC_SRCS})
TARGET_LINK_LIBRARIES(oracle ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES})

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
    # this needs to be relative
    set(plugin_dest_dir ./oracle.app/Contents/Plugins)
    # instead, this needs an absolute path (for make)
    set(qtconf_dest_dir ${CMAKE_INSTALL_PREFIX}/oracle.app/Contents/Resources)

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
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/oracle.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()
