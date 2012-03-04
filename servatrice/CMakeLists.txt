CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(servatrice)

SET(servatrice_SOURCES
 src/main.cpp
 src/passwordhasher.cpp
 src/servatrice.cpp 
 src/server_logger.cpp 
 src/serversocketinterface.cpp 
 src/serversocketthread.cpp
 src/networkserverinterface.cpp
 src/networkserverthread.cpp
 ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
)
SET(servatrice_HEADERS
 src/servatrice.h 
 src/server_logger.h 
 src/serversocketinterface.h 
 src/serversocketthread.h
 src/networkserverinterface.h
 src/networkserverthread.h
)

SET(QT_DONTUSE_QTGUI)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSQL TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(Libgcrypt REQUIRED)

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
TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${QT_LIBRARIES} ${LIBGCRYPT_LIBRARY})

#add_custom_target(versionheader ALL DEPENDS version_header)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version_string.h ${CMAKE_CURRENT_BINARY_DIR}/version_string.cpp
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../common/getversion.cmake
)

