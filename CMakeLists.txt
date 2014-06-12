# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation
# of the different projects: servatrice, cockatrice, test
# This file sets all the variables shared between the projects
# like the installation path, compilation flags etc..

cmake_minimum_required(VERSION 2.6)

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

# Force "Release" build type by default
set(CMAKE_BUILD_TYPE Release)

# Define proper compilation flags
IF (CMAKE_COMPILER_IS_GNUCC)
    # linux/gcc, bsd/gcc, windows/mingw
    set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0")
else()
    # other: osx/llvm, bsd/llvm
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
ENDIF (CMAKE_COMPILER_IS_GNUCC)

# GNU systems need to define the Mersenne exponent for the RNG to compile w/o warning
IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    ADD_DEFINITIONS("-DSFMT_MEXP=19937")
ENDIF()

#Find Qt4 and enable the needed features
FIND_PACKAGE(Qt4 REQUIRED)

set(CMAKE_AUTOMOC TRUE)

# Find other needed libraries
FIND_PACKAGE(Protobuf REQUIRED)

# Compile servatrice (default off)
option(WITH_SERVER "build servatrice" OFF)
add_subdirectory(common)
if(WITH_SERVER)
    add_subdirectory(servatrice)
endif()

# Compile cockatrice (default on)
option(WITH_CLIENT "build cockatrice" ON)
if(WITH_CLIENT)
    add_subdirectory(cockatrice)
    add_subdirectory(sounds)
    add_subdirectory(zonebg)
endif()

# Compile oracle (default on)
option(WITH_ORACLE "build oracle" ON)
if(WITH_ORACLE)
    add_subdirectory(oracle)
endif()

# Compile testclient (default off)
option(WITH_TESTCLIENT "build testclient" OFF)
if (WITH_TESTCLIENT)
    add_subdirectory(testclient)
endif()