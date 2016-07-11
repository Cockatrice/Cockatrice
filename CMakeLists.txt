# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation
# of the different projects: servatrice, cockatrice, test
# This file sets all the variables shared between the projects
# like the installation path, compilation flags etc..

# Cmake 3.1 is required to enable C++11 support correctly
cmake_minimum_required(VERSION 3.1)

if(POLICY CMP0020)
    cmake_policy(SET CMP0020 OLD)
endif()

if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD)
endif()

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 OLD)
endif()

if(POLICY CMP0064)
    cmake_policy(SET CMP0064 OLD)
endif()

set(PROJECT_NAME "Cockatrice")

# Default to "Release" build type
# User-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Type of build")
ELSE()
   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Type of build")
ENDIF()

# A project name is needed for CPack
PROJECT("${PROJECT_NAME}")

# Use c++11 for all targets
set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ ISO Standard")
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set conventional loops
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Search path for cmake modules
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Retrieve git version hash
include(getversion)

# Create a header and a cpp file containing the version hash
include(createversionfile)

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
    # Support from Windows XP
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
	# Maximum optimization
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD")
	# Generate complete debugging information
    #set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
ELSEIF (CMAKE_COMPILER_IS_GNUCXX)
    # linux/gcc, bsd/gcc, windows/mingw
    include(CheckCXXCompilerFlag)

    set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -Werror")

    set(ADDITIONAL_DEBUG_FLAGS -Wcast-align -Wmissing-declarations -Wno-long-long -Wno-error=extra -Wno-error=delete-non-virtual-dtor -Wno-error=sign-compare -Wno-error=missing-declarations)

    FOREACH(FLAG ${ADDITIONAL_DEBUG_FLAGS})
        CHECK_CXX_COMPILER_FLAG("${FLAG}" CXX_HAS_WARNING_${FLAG})
        IF(CXX_HAS_WARNING_${FLAG})
            SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${FLAG}")
        ENDIF()
    ENDFOREACH()
ELSE()
    # other: osx/llvm, bsd/llvm
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
ENDIF()

# GNU systems need to define the Mersenne exponent for the RNG to compile w/o warning
IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    ADD_DEFINITIONS("-DSFMT_MEXP=19937")
ENDIF()

# Find Qt5
OPTION(UPDATE_TRANSLATIONS "Update translations on compile" OFF)
MESSAGE(STATUS "UPDATE TRANSLATIONS: ${UPDATE_TRANSLATIONS}")

IF(WIN32)
    FIND_PACKAGE(Qt5Widgets 5.4.0 REQUIRED)  # For QSysInfo::buildAbi()
ELSE()
    FIND_PACKAGE(Qt5Widgets 5.0.3 REQUIRED)
ENDIF()

IF(Qt5Widgets_FOUND)
    MESSAGE(STATUS "Found Qt ${Qt5Widgets_VERSION_STRING}")

    # FIX: Qt was built with -reduce-relocations
    if (Qt5_POSITION_INDEPENDENT_CODE)
        SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()

    FIND_PACKAGE(Qt5LinguistTools)
    IF(UPDATE_TRANSLATIONS)
        IF(NOT Qt5_LUPDATE_EXECUTABLE)
            MESSAGE(WARNING "Qt's lupdate not found.")
        ENDIF()
    ENDIF()

    IF(NOT Qt5_LRELEASE_EXECUTABLE)
        MESSAGE(WARNING "Qt's lrelease not found.")
    ENDIF()

ELSE()
    MESSAGE(FATAL_ERROR "No Qt5 found!")
ENDIF()

set(CMAKE_AUTOMOC TRUE)

# Find other needed libraries
FIND_PACKAGE(Protobuf REQUIRED)

#Find OpenSSL
IF(WIN32)
    FIND_PACKAGE(Win32SslRuntime)
ENDIF()

# Package builder
set(CPACK_PACKAGE_CONTACT "Gavin Bisesi <Daenyth+github@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Cockatrice Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
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
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
    	set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    	set(CPACK_DEBIAN_PACKAGE_SECTION "games")
    	set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://github.com/Cockatrice/Cockatrice")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5-plugins, libqt5svg5")
    endif()
elseif(WIN32)
    set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

    # Configure file with custom definitions for NSIS.
    configure_file(
        ${CMAKE_MODULE_PATH}/NSIS.definitions.nsh.in
        ${PROJECT_BINARY_DIR}/NSIS.definitions.nsh
    )
endif()

# Configure file with build deployment data for travis
configure_file(
    ${CMAKE_MODULE_PATH}/bintray_deploy.json.in
    ${PROJECT_BINARY_DIR}/bintray_deploy.json
)

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
    SET(CPACK_INSTALL_CMAKE_PROJECTS "cockatrice;cockatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile oracle (default on)
option(WITH_ORACLE "build oracle" ON)
if(WITH_ORACLE)
    add_subdirectory(oracle)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "release/oracle.app;oracle;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile tests (default off)
option(TEST "build tests" OFF)
if(TEST)
    include(CTest)
    add_subdirectory(tests)
endif()
