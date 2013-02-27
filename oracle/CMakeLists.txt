CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(oracle)

# paths
set(DESKTOPDIR share/applications CACHE STRING "path to .desktop files")

SET(oracle_SOURCES src/main.cpp src/oracleimporter.cpp src/window_main.cpp ../cockatrice/src/carddatabase.cpp ../cockatrice/src/settingscache.cpp)
SET(oracle_HEADERS src/oracleimporter.h src/window_main.h ../cockatrice/src/carddatabase.h ../cockatrice/src/settingscache.h)

SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTSVG TRUE)
FIND_PACKAGE(Qt4 REQUIRED)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")

QT4_WRAP_CPP(oracle_HEADERS_MOC ${oracle_HEADERS})
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(../cockatrice/src)

ADD_EXECUTABLE(oracle WIN32 MACOSX_BUNDLE ${oracle_SOURCES} ${oracle_HEADERS_MOC})
TARGET_LINK_LIBRARIES(oracle ${QT_LIBRARIES})

IF (NOT APPLE)
  INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/oracle DESTINATION bin)
ELSE (APPLE)
  INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/oracle.app DESTINATION bin)
ENDIF (NOT APPLE)
IF (NOT WIN32 AND NOT APPLE)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oracle.desktop DESTINATION ${DESKTOPDIR})
ENDIF (NOT WIN32 AND NOT APPLE)
