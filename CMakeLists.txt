# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation
# of the different projects: servatrice, cockatrice, test
# This file sets all the variables shared between the projects
# like the installation path, compilation flags etc..

cmake_minimum_required(VERSION 2.6)

if(POLICY CMP0020)
    cmake_policy(SET CMP0020 OLD)
endif()

if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD)
endif()

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

set(PROJECT_NAME "Cockatrice")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

# Defualt to "Release" build type
# User-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Type of build")
ELSE()
   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Type of build")
ENDIF()

# A project name is needed for CPack
PROJECT("${PROJECT_NAME}")

# Set conventional loops
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Define a proper install path
if(UNIX)
    if(APPLE)
        # MacOS X
        # Due to the special bundle structure ignore
        # the prefix eventually set by the user.
        set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release)
    else()
        # Linux / BSD
        if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
            #fix package build
            if(PREFIX)
                set(CMAKE_INSTALL_PREFIX ${PREFIX})
            else()
                set(CMAKE_INSTALL_PREFIX /usr/local)
            endif()
        endif()
    endif()
elseif(WIN32)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release)
endif()

# Define proper compilation flags
IF(MSVC)
	# Visual Studio:
	# Maximum optimization
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox")
	# Generate complete debugging information
    #set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
ELSEIF (CMAKE_COMPILER_IS_GNUCXX)
    # linux/gcc, bsd/gcc, windows/mingw
    set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0")
ELSE()
    # other: osx/llvm, bsd/llvm
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
ENDIF()

# GNU systems need to define the Mersenne exponent for the RNG to compile w/o warning
IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    ADD_DEFINITIONS("-DSFMT_MEXP=19937")
ENDIF()

#Find Qt and enable the needed features
FIND_PACKAGE(Qt5Widgets)
IF(Qt5Widgets_FOUND)
    MESSAGE(STATUS "Found Qt ${Qt5Widgets_VERSION_STRING}")
ELSE()
    FIND_PACKAGE(Qt4 4.8.0 REQUIRED)
    IF(QT4_FOUND)
        # Old FindQt4.cmake modules used the same flag with different case
        SET(Qt4_FOUND TRUE)
    ENDIF()
    IF(Qt4_FOUND)
        MESSAGE(STATUS "Found Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
    ELSE()
        MESSAGE(FATAL_ERROR "No Qt4 or Qt5 found!")
    ENDIF()
ENDIF()

set(CMAKE_AUTOMOC TRUE)

# Find other needed libraries
FIND_PACKAGE(Protobuf REQUIRED)

# Package builder
set(CPACK_PACKAGE_CONTACT "Daenyth+github@gmail.com") 
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Cockatrice Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

if(UNIX)
    if(APPLE)
        set(CPACK_GENERATOR DragNDrop ${CPACK_GENERATOR})
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_DMG_FORMAT "UDBZ")
        set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
        set(CPACK_SYSTEM_NAME "OSX")
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice/resources/appicon.icns")
    else()
        # linux
        set(CPACK_GENERATOR DEB ${CPACK_GENERATOR})
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}/${PROJECT_VERSION}")
        set(CPACK_STRIP_FILES "bin/${PROJECT_NAME}")
        set(CPACK_SOURCE_STRIP_FILES "")
    endif()
elseif(WIN32)
    set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}\\\\${PROJECT_VERSION}")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PROJECT_NAME}")
    set(CPACK_NSIS_HELP_LINK "https://github.com/Daenyth/Cockatrice")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Daenyth/Cockatrice")
    set(CPACK_NSIS_CONTACT "Daenyth+github@gmail.com")
    set(CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)

# Compile servatrice (default off)
option(WITH_SERVER "build servatrice" OFF)
add_subdirectory(common)
if(WITH_SERVER)
    add_subdirectory(servatrice)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "servatrice;servatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile cockatrice (default on)
option(WITH_CLIENT "build cockatrice" ON)
if(WITH_CLIENT)
    add_subdirectory(cockatrice)
    add_subdirectory(sounds)
    add_subdirectory(zonebg)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "cockatrice;cockatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile oracle (default on)
option(WITH_ORACLE "build oracle" ON)
if(WITH_ORACLE)
    add_subdirectory(oracle)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "release/oracle.app;oracle;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()
