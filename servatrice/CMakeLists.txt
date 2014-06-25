# CMakeLists for servatrice directory
#
# provides the servatrice binary

PROJECT(servatrice)

# cmake module for libgcrypt is included in current directory
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
FIND_PACKAGE(Libgcrypt REQUIRED)

SET(servatrice_SOURCES
    src/main.cpp
    src/passwordhasher.cpp
    src/servatrice.cpp
    src/servatrice_connection_pool.cpp
    src/servatrice_database_interface.cpp
    src/server_logger.cpp
    src/serversocketinterface.cpp
    src/isl_interface.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
)

SET(QT_DONTUSE_QTGUI)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSQL TRUE)

# Include directories
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${LIBGCRYPT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Build servatrice binary and link it
ADD_EXECUTABLE(servatrice MACOSX_BUNDLE ${servatrice_SOURCES} ${servatrice_MOC_SRCS})
TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${QT_LIBRARIES} ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

#add_custom_target(versionheader ALL DEPENDS version_header)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_string.h ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/getversion.cmake
)

# install rules
if(UNIX)
    if(APPLE)
        INSTALL(TARGETS servatrice BUNDLE DESTINATION ./)
    else()
        # Assume linux
        INSTALL(TARGETS servatrice RUNTIME DESTINATION bin/)
    endif()
elseif(WIN32)
    INSTALL(TARGETS servatrice RUNTIME DESTINATION ./)
endif()

if(APPLE)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir servatrice.app/Contents/Plugins)
    set(qtconf_dest_dir servatrice.app/Contents/Resources)

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
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations\")
    " COMPONENT Runtime)

    install(CODE "
        file(GLOB_RECURSE QTPLUGINS
        \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    include(BundleUtilities)
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()
