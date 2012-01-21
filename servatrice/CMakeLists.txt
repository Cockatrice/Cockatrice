CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(servatrice)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../common ${CMAKE_CURRENT_BINARY_DIR}/common)

SET(servatrice_SOURCES
 src/main.cpp
 src/passwordhasher.cpp
 src/servatrice.cpp 
 src/server_logger.cpp 
 src/serversocketinterface.cpp 
 src/serversocketthread.cpp
)
SET(servatrice_HEADERS
 src/servatrice.h 
 src/server_logger.h 
 src/serversocketinterface.h 
 src/serversocketthread.h
)

SET(QT_DONTUSE_QTGUI)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSQL TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(Libgcrypt REQUIRED)
set(CMAKE_BUILD_TYPE Release)

QT4_WRAP_CPP(servatrice_HEADERS_MOC ${servatrice_HEADERS})

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../common)
INCLUDE_DIRECTORIES(${LIBGCRYPT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../common)

ADD_EXECUTABLE(servatrice ${servatrice_SOURCES} ${servatrice_HEADERS_MOC})
TARGET_LINK_LIBRARIES(servatrice cockatrice_common ${QT_LIBRARIES} ${PROTOBUF_LIBRARIES} ${LIBGCRYPT_LIBRARY})
