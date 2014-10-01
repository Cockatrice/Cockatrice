# CMakeLists for servatrice directory
#
# provides the servatrice binary

PROJECT(servatrice)

SET(servatrice_SOURCES
    src/main.cpp
    src/passwordhasher.cpp
    src/servatrice.cpp
    src/servatrice_connection_pool.cpp
    src/servatrice_database_interface.cpp
    src/server_logger.cpp
    src/serversocketinterface.cpp
    src/isl_interface.cpp
    ${VERSION_STRING_CPP}
)

set(SERVATRICE_LIBS)

# Qt4 stuff
if(Qt4_FOUND)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTSQL TRUE)

    # Include directories
    INCLUDE(${QT_USE_FILE})
    include_directories(${QT_INCLUDES})
    LIST(APPEND SERVATRICE_LIBS ${QT_LIBRARIES})

    # Libgcrypt is required only with Qt4 to support SHA512 hashing
    FIND_PACKAGE(Libgcrypt REQUIRED)
    INCLUDE_DIRECTORIES(${LIBGCRYPT_INCLUDE_DIR})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND SERVATRICE_LIBS Widgets)

    # QtNetwork
    find_package(Qt5Network)
    if(Qt5Network_FOUND)
        include_directories(${Qt5Network_INCLUDE_DIRS})
        list(APPEND SERVATRICE_LIBS Network)
    endif()

    # QtSql
    find_package(Qt5Sql)
    if(Qt5Sql_FOUND)
        include_directories(${Qt5Sql_INCLUDE_DIRS})
        list(APPEND SERVATRICE_LIBS Sql)
    endif()

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)
endif()

SET(QT_DONT_USE_QTGUI TRUE)

# Include directories
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Build servatrice binary and link it
ADD_EXECUTABLE(servatrice MACOSX_BUNDLE ${servatrice_SOURCES} ${servatrice_MOC_SRCS})

if(Qt4_FOUND)
    if(MSVC) 
        set(QT_USE_QTMAIN true) 
    endif() 
    TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${SERVATRICE_LIBS} ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
endif()
if(Qt5Widgets_FOUND)
    if(MSVC)
        TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} Qt5::WinMain)
    else()
        TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
    endif()
    qt5_use_modules(servatrice ${SERVATRICE_LIBS})
endif()

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

if(WIN32)
    # these needs to be relative to CMAKE_INSTALL_PREFIX
    set(plugin_dest_dir Plugins)
    set(qtconf_dest_dir .)

    # note: no codecs in qt5
    # note: phonon_backend => mediaservice
    # note: needs platform on osx

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/.*d\\.dll")
    else()
        install(DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
            FILES_MATCHING REGEX "(codecs|iconengines|imageformats|mediaservice|phonon_backend|platforms)/.*[^d]\\.dll")
    endif()

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
    fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/servatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
    " COMPONENT Runtime)
endif()