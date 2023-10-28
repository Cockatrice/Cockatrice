# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation
# of the different projects: servatrice, cockatrice, test
# This file sets all the variables shared between the projects
# like the installation path, compilation flags etc..

# cmake 3.16 is required if using qt6
cmake_minimum_required(VERSION 3.10)

# Early detect ccache
option(USE_CCACHE "Cache the build results with ccache" ON)
# Treat warnings as errors (Debug builds only)
option(WARNING_AS_ERROR "Treat warnings as errors in debug builds" ON)
# Check for translation updates
option(UPDATE_TRANSLATIONS "Update translations on compile" OFF)
# Compile servatrice
option(WITH_SERVER "build servatrice" OFF)
# Compile cockatrice
option(WITH_CLIENT "build cockatrice" ON)
# Compile oracle
option(WITH_ORACLE "build oracle" ON)
# Compile dbconverter
option(WITH_DBCONVERTER "build dbconverter" ON)
# Compile tests
option(TEST "build tests" OFF)

# Default to "Release" build type
# User-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
if(DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      ${CMAKE_BUILD_TYPE}
      CACHE STRING "Type of build"
  )
else()
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Type of build"
  )
endif()

if(USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    # Support Unix Makefiles and Ninja
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    message(STATUS "Found CCache ${CCACHE_PROGRAM}")
  endif()
endif()

if(WIN32)
  # Use vcpkg toolchain on Windows
  set(CMAKE_TOOLCHAIN_FILE
      ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake
      CACHE STRING "Vcpkg toolchain file"
  )
  # Qt path set by user or env var
  if(QTDIR
     OR DEFINED ENV{QTDIR}
     OR DEFINED ENV{QTDIR32}
     OR DEFINED ENV{QTDIR64}
  )

  else()
    set(QTDIR
        ""
        CACHE PATH "Path to Qt (e.g. C:/Qt/5.7/msvc2015_64)"
    )
    message(
      WARNING "QTDIR variable is missing.  Please set this variable to specify path to Qt (e.g. C:/Qt/5.7/msvc2015_64)"
    )
  endif()
endif()

# A project name is needed for CPack
# Version can be overriden by git tags, see cmake/getversion.cmake
project("Cockatrice" VERSION 2.9.0)

# Set release name if not provided via env/cmake var
if(NOT DEFINED GIT_TAG_RELEASENAME)
  set(GIT_TAG_RELEASENAME "Rings of the Wild")
endif()

# Use c++17 for all targets
set(CMAKE_CXX_STANDARD
    17
    CACHE STRING "C++ ISO Standard"
)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set conventional loops
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Search path for cmake modules
set(COCKATRICE_CMAKE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(INSERT CMAKE_MODULE_PATH 0 "${COCKATRICE_CMAKE_PATH}")

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
      message(STATUS "Force enabling CCache usage under macOS")
      # Set up wrapper scripts
      configure_file("${COCKATRICE_CMAKE_PATH}/launch-c.in" launch-c)
      configure_file("${COCKATRICE_CMAKE_PATH}/launch-cxx.in" launch-cxx)
      execute_process(COMMAND chmod a+rx "${CMAKE_BINARY_DIR}/launch-c" "${CMAKE_BINARY_DIR}/launch-cxx")

      # Set Xcode project attributes to route compilation through our scripts
      set(CMAKE_XCODE_ATTRIBUTE_CC "${CMAKE_BINARY_DIR}/launch-c")
      set(CMAKE_XCODE_ATTRIBUTE_CXX "${CMAKE_BINARY_DIR}/launch-cxx")
      set(CMAKE_XCODE_ATTRIBUTE_LD "${CMAKE_BINARY_DIR}/launch-c")
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
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/rundir/${CMAKE_BUILD_TYPE})
endif()

# Define proper compilation flags
if(MSVC)
  # Visual Studio: Maximum optimization, disable warning C4251, establish C++17 compatibility
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD /wd4251 /Zc:__cplusplus /std:c++17 /permissive- /W4")
  # Generate complete debugging information
  #set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  # linux/gcc, bsd/gcc, windows/mingw
  include(CheckCXXCompilerFlag)

  set(CMAKE_CXX_FLAGS_RELEASE "-s -O2")
  if(WARNING_AS_ERROR)
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -Werror")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra")
  endif()

  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17")
  endif()

  set(ADDITIONAL_DEBUG_FLAGS
      -Wcast-align
      -Wmissing-declarations
      -Wno-long-long
      -Wno-error=extra
      -Wno-error=delete-non-virtual-dtor
      -Wno-error=sign-compare
      -Wno-error=missing-declarations
  )

  foreach(FLAG ${ADDITIONAL_DEBUG_FLAGS})
    check_cxx_compiler_flag("${FLAG}" CXX_HAS_WARNING_${FLAG})
    if(CXX_HAS_WARNING_${FLAG})
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${FLAG}")
    endif()
  endforeach()
else()
  # other: osx/llvm, bsd/llvm
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
  if(WARNING_AS_ERROR)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror -Wno-unused-parameter")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
  endif()
endif()

# GNU systems need to define the Mersenne exponent for the RNG to compile w/o warning
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions("-DSFMT_MEXP=19937")
endif()

find_package(Threads REQUIRED)

# Determine 32 or 64 bit build
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_lib_suffix 64)
else()
  set(_lib_suffix 32)
endif()

if(DEFINED QTDIR${_lib_suffix})
  list(APPEND CMAKE_PREFIX_PATH "${QTDIR${_lib_suffix}}")
elseif(DEFINED QTDIR)
  list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
elseif(DEFINED ENV{QTDIR${_lib_suffix}})
  list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR${_lib_suffix}}")
elseif(DEFINED ENV{QTDIR})
  list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

message(STATUS "Update Translations: ${UPDATE_TRANSLATIONS}")

include(FindQtRuntime)

set(CMAKE_AUTOMOC TRUE)

# Find other needed libraries
find_package(Protobuf CONFIG)
if(NOT Protobuf_FOUND)
  find_package(Protobuf REQUIRED)
endif()

if(${Protobuf_VERSION} VERSION_LESS "3.21.0.0" AND NOT EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
  message(FATAL_ERROR "No protoc command found!")
endif()

#Find OpenSSL
if(WIN32)
  find_package(OpenSSL REQUIRED)
  if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
  else()
    message(
      WARNING
        "Could not find OpenSSL runtime libraries. They are not required for compiling, but needs to be available at runtime."
    )
  endif()
endif()

#Find VCredist
if(MSVC)
  find_package(VCredistRuntime)
endif()

# Package builder
set(CPACK_PACKAGE_CONTACT "Zach Halpern <zach@cockatrice.us>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}")
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
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeDMGSetup.script")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dmgBackground.tif")
  else()
    # linux
    if(CPACK_GENERATOR STREQUAL "RPM")
      set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
      set(CPACK_RPM_MAIN_COMPONENT "cockatrice")
      if(Qt6_FOUND)
        set(CPACK_RPM_PACKAGE_REQUIRES "protobuf, qt6-qttools, qt6-qtsvg, qt6-qtmultimedia")
      elseif(Qt5_FOUND)
        set(CPACK_RPM_PACKAGE_REQUIRES "protobuf, qt5-qttools, qt5-qtsvg, qt5-qtmultimedia")
      endif()
      set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
      set(CPACK_RPM_PACKAGE_URL "http://github.com/Cockatrice/Cockatrice")
      # stop directories from making package conflicts
      set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
          /usr/share/applications
          /usr/share/icons
          /usr/share/icons/hicolor
          /usr/share/icons/hicolor/48x48
          /usr/share/icons/hicolor/48x48/apps
          /usr/share/icons/hicolor/scalable
          /usr/share/icons/hicolor/scalable/apps
      )
    else()
      set(CPACK_GENERATOR DEB)
      set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
      set(CPACK_DEBIAN_PACKAGE_SECTION "games")
      set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://github.com/Cockatrice/Cockatrice")
      if(Qt6_FOUND)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6multimedia6, libqt6svg6, qt6-qpa-plugins")
      elseif(Qt5_FOUND)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5multimedia5-plugins, libqt5svg5")
      endif()
    endif()
  endif()
elseif(WIN32)
  set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
  if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "(x64)")
    set(TRICE_IS_64_BIT 1)
  else()
    set(TRICE_IS_64_BIT 0)
  endif()

  # Configure file with custom definitions for NSIS.
  configure_file("${COCKATRICE_CMAKE_PATH}/NSIS.definitions.nsh.in" "${PROJECT_BINARY_DIR}/NSIS.definitions.nsh")

  # include vcredist into the package; NSIS will take care of running it
  if(VCREDISTRUNTIME_FOUND)
    install(FILES "${VCREDISTRUNTIME_FILE}" DESTINATION ./)
  endif()
endif()

include(CPack)

add_subdirectory(common)
if(WITH_SERVER)
  add_subdirectory(servatrice)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Servatrice;Servatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(WITH_CLIENT)
  add_subdirectory(cockatrice)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Cockatrice;Cockatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(WITH_ORACLE)
  add_subdirectory(oracle)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Oracle;Oracle;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(WITH_DBCONVERTER)
  add_subdirectory(dbconverter)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Dbconverter;Dbconverter;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(TEST)
  include(CTest)
  add_subdirectory(tests)
endif()

if(Qt6_FOUND AND Qt6_VERSION_MINOR GREATER_EQUAL 3)
  # Qt6.3+ requires project finalization to support translations
  qt6_finalize_project()
endif()
