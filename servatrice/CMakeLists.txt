CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(servatrice)

# cmake modules
include(GNUInstallDirs)

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
SET(servatrice_HEADERS
 src/servatrice.h 
 src/servatrice_connection_pool.h
 src/servatrice_database_interface.h
 src/server_logger.h 
 src/serversocketinterface.h 
 src/isl_interface.h
)

SET(QT_DONTUSE_QTGUI)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSQL TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(Libgcrypt REQUIRED)
FIND_PACKAGE(Threads)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

QT4_WRAP_CPP(servatrice_HEADERS_MOC ${servatrice_HEADERS})

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${LIBGCRYPT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(servatrice ${servatrice_SOURCES} ${servatrice_HEADERS_MOC})
TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${QT_LIBRARIES} ${LIBGCRYPT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

#add_custom_target(versionheader ALL DEPENDS version_header)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_string.h ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/getversion.cmake
)

# install rules
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/servatrice DESTINATION ${CMAKE_INSTALL_BINDIR})
