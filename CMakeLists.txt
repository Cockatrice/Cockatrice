# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation
# of the different projects: servatrice, cockatrice, test
# This file sets all the variables shared between the projects
# like the installation path, compilation flags etc..

# Cmake 3.1 is required to enable C++11 support correctly
cmake_minimum_required(VERSION 3.1)

if(POLICY CMP0064)
    cmake_policy(SET CMP0064 NEW)
endif()

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

# Default to "Release" build type
# User-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Type of build")
ELSE()
   SET(CMAKE_BUILD_TYPE Release CACHE STRING "Type of build")
ENDIF()

# Early detect ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    # Support Unix Makefiles and Ninja
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    MESSAGE(STATUS "Found CCache ${CCACHE_PROGRAM}")
endif()

# A project name is needed for CPack
# Version can be overriden by git tags, see cmake/getversion.cmake
PROJECT("Cockatrice" VERSION 2.7.0)

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
        # macOS
        # Due to the special bundle structure ignore
        # the prefix eventually set by the user.
        set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release)

        # Force ccache usage if available
        get_property(RULE_LAUNCH_COMPILE GLOBAL PROPERTY RULE_LAUNCH_COMPILE)
        if(RULE_LAUNCH_COMPILE)
            MESSAGE(STATUS "Force enabling CCache usage under macOS")
            # Set up wrapper scripts
            configure_file(${CMAKE_MODULE_PATH}/launch-c.in   launch-c)
            configure_file(${CMAKE_MODULE_PATH}/launch-cxx.in launch-cxx)
            execute_process(COMMAND chmod a+rx
                "${CMAKE_BINARY_DIR}/launch-c"
                "${CMAKE_BINARY_DIR}/launch-cxx")

            # Set Xcode project attributes to route compilation through our scripts
            set(CMAKE_XCODE_ATTRIBUTE_CC         "${CMAKE_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_CXX        "${CMAKE_BINARY_DIR}/launch-cxx")
            set(CMAKE_XCODE_ATTRIBUTE_LD         "${CMAKE_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${CMAKE_BINARY_DIR}/launch-cxx")
        endif()
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

# Treat warnings as errors (Debug builds only)
option(WARNING_AS_ERROR "Treat warnings as errors in debug builds" ON)

# Define proper compilation flags
IF(MSVC)
    # Visual Studio:
    # Maximum optimization
    # Disable warning C4251
    set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD /wd4251")
    # Generate complete debugging information
    #set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
ELSEIF (CMAKE_COMPILER_IS_GNUCXX)
    # linux/gcc, bsd/gcc, windows/mingw
    include(CheckCXXCompilerFlag)

    set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")
    if(WARNING_AS_ERROR)
        set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -Werror")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra")
    endif()

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

FIND_PACKAGE(Qt5Core 5.5.0 REQUIRED)

IF(Qt5Core_FOUND)
    MESSAGE(STATUS "Found Qt ${Qt5Core_VERSION_STRING}")

    # FIX: Qt was built with -reduce-relocations
    if (Qt5_POSITION_INDEPENDENT_CODE)
        SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()

    # guess plugins and libraries directory
    set(QT_PLUGINS_DIR "${Qt5Core_DIR}/../../../plugins")
    get_target_property(QT_LIBRARY_DIR Qt5::Core LOCATION)
    get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)

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

#Find VCredist
IF(MSVC)
    FIND_PACKAGE(VCredistRuntime)
ENDIF()

# Package builder
set(CPACK_PACKAGE_CONTACT "Zach Halpern <zahalpern+github@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Cockatrice Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_VERSION_FILENAME}")

if(UNIX)
    if(APPLE)
        set(CPACK_GENERATOR DragNDrop ${CPACK_GENERATOR})
        set(CPACK_GENERATOR "DragNDrop")
        set(CPACK_DMG_FORMAT "UDBZ")
        set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
        set(CPACK_SYSTEM_NAME "OSX")
        set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice/resources/appicon.icns")
    else()
        # linux
        IF(CPACK_GENERATOR STREQUAL "RPM")
            set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
            set(CPACK_RPM_PACKAGE_REQUIRES "protobuf, qt5-qttools, qt5-qtsvg, qt5-qtmultimedia")
            set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
            set(CPACK_RPM_PACKAGE_URL "http://github.com/Cockatrice/Cockatrice")
        ELSE()
            set(CPACK_GENERATOR DEB)
            set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
            set(CPACK_DEBIAN_PACKAGE_SECTION "games")
            set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://github.com/Cockatrice/Cockatrice")
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5-plugins, libqt5svg5")
        ENDIF()
    endif()
elseif(WIN32)
    set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
    if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
        set(TRICE_IS_64_BIT 1)
    else()
        set(TRICE_IS_64_BIT 0)
    endif()

    # Configure file with custom definitions for NSIS.
    configure_file(
        ${CMAKE_MODULE_PATH}/NSIS.definitions.nsh.in
        ${PROJECT_BINARY_DIR}/NSIS.definitions.nsh
    )

    # include vcredist into the package; NSIS will take care of running it
    if(VCREDISTRUNTIME_FOUND)
        INSTALL(FILES "${VCREDISTRUNTIME_FILE}" DESTINATION ./)
    endif()
endif()

include(CPack)

# Compile servatrice (default off)
option(WITH_SERVER "build servatrice" OFF)
add_subdirectory(common)
if(WITH_SERVER)
    add_subdirectory(servatrice)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "Servatrice;Servatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile cockatrice (default on)
option(WITH_CLIENT "build cockatrice" ON)
if(WITH_CLIENT)
    add_subdirectory(cockatrice)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "Cockatrice;Cockatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile oracle (default on)
option(WITH_ORACLE "build oracle" ON)
if(WITH_ORACLE)
    add_subdirectory(oracle)
    SET(CPACK_INSTALL_CMAKE_PROJECTS "Oracle;Oracle;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

# Compile tests (default off)
option(TEST "build tests" OFF)
if(TEST)
    include(CTest)
    add_subdirectory(tests)
endif()
