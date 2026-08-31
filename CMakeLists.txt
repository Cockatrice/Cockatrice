# Cockatrice's main CMakeLists.txt
#
# This is basically a wrapper to enable/disable the compilation of the different projects:
# Cockatrice, Oracle, Servatrice, Test
# This file sets all the variables shared between the projects like the installation path, compilation flags etc..

# CMake 3.16 is required if using Qt6
cmake_minimum_required(VERSION 3.10)

# Compile Cockatrice
option(WITH_CLIENT "Build Cockatrice client" ON)
# Compile Oracle
option(WITH_ORACLE "Build Cockatrice card database tool (Oracle)" ON)
# Compile Servatrice
option(WITH_SERVER "Build Cockatrice server (Servatrice)" OFF)
# Compile Tests
option(TEST "Build tests" OFF)

# Check for translation updates
option(UPDATE_TRANSLATIONS "Update translations on compile" OFF)
# Use vcpkg regardless of OS
option(USE_VCPKG "Use vcpkg regardless of OS" OFF)
# Use compiler cache (ccache)
option(USE_CCACHE "Cache the build results with ccache" OFF)
# Treat warnings as errors (Debug builds only)
option(WARNING_AS_ERROR "Treat warnings as errors in debug builds" ON)

# Default to "Release" build type
# User-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type"
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

if(WIN32 OR USE_VCPKG)
  # Use vcpkg toolchain on Windows (and on macOS in CI)
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
        CACHE PATH "Path to Qt (e.g. C:/Qt/6.4.2/msvc2019_64)"
    )
    message(
      WARNING
        "QTDIR variable is missing.  Please set this variable to specify path to Qt (e.g. C:/Qt/6.4.2/msvc2019_64)"
    )
  endif()
endif()

# A project name is needed for CPack
# Version can be overriden by git tags, see cmake/getversion.cmake
project("Cockatrice" VERSION 3.1.0)

# Set release name if not provided via ENV/CMake var
if(NOT DEFINED GIT_TAG_RELEASENAME)
  set(GIT_TAG_RELEASENAME "Graduation Day")
endif()

# Use C++20 for all targets
set(CMAKE_CXX_STANDARD
    20
    CACHE STRING "C++ ISO Standard"
)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# try target_compile_features(cockatrice PRIVATE cxx_std_20)
# -fexperimental-library for gcc/clang, libstdc++,  gate with compiler_is_gnucxx or cxx_compiler_id matches clang

# Set conventional loops
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Search path for CMake modules
set(COCKATRICE_CMAKE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(INSERT CMAKE_MODULE_PATH 0 "${COCKATRICE_CMAKE_PATH}")

include(getversion)

# Create a header and a cpp file containing the version hash
include(createversionfile)

# Define a proper install path
if(UNIX)
  if(APPLE) # macOS
    # Due to the special bundle structure ignore the prefix eventually set by the user
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release)

    # Force ccache usage if available
    if(USE_CCACHE)
      get_property(RULE_LAUNCH_COMPILE GLOBAL PROPERTY RULE_LAUNCH_COMPILE)
      if(RULE_LAUNCH_COMPILE)
        message(STATUS "Force enabling ccache usage")

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
    endif()
  else() # Linux / BSD
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      # Fix package build
      if(PREFIX)
        set(CMAKE_INSTALL_PREFIX ${PREFIX})
      else()
        set(CMAKE_INSTALL_PREFIX /usr/local)
      endif()
    endif()
  endif()
elseif(WIN32) # Windows (including 64bit)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/rundir/${CMAKE_BUILD_TYPE})
endif()

# Define compiler flags
if(MSVC) # MS Visual C++ compiler
  # /EHsc              Enable standard C++ exception handling
  # /MP                Enable parallel compilation
  # /permissive-       Enable strict standards compliance
  # /utf-8             Set source file encoding and execution char set to UTF-8
  # /W4                Enable warning level 4
  # /Zc:__cplusplus    Enable C++20 detection in headers
  # /Zi                Generate debugging information (Program Database, PDB)
  set(CMAKE_CXX_FLAGS "/EHsc /MP /permissive- /utf-8 /W4 /Zc:__cplusplus /Zi")

  # /Ox                Enable maximum optimization
  # /MD                Link against the multi-threaded DLL runtime library (Release CRT)
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /MD")

  # /Od                Disable optimization
  # /MDd               Link against the multi-threaded Debug DLL runtime library (Debug CRT)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /MDd")

  # Generate PDBs, even when building release target to allow developers to better analyze crash logs
  # /DEBUG             Enable PDB generation also for Release builds
  # /OPT:REF           Remove unused references
  # /OPT:ICF           Fold identical code
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

elseif(CMAKE_COMPILER_IS_GNUCXX) # Linux/GCC, BSD/GCC, Windows/MinGW
  include(CheckCXXCompilerFlag)

  # -O2             Balanced optimization
  # -s              Remove debug info from the binary (strip symbols)
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -s")

  # -ggdb           Produce GDB debugging symbols
  # -O0             No optimization
  # -Wall           Enable all warnings
  # -Wextra         Enable extra warnings
  # -Werror         Treat warnings as compilation errors
  if(WARNING_AS_ERROR)
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -Werror")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra")
  endif()

  # -std=gnu++20    Enable GNU C++20 extensions
  if(APPLE) # macOS/GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++20")
  endif()

  # -Wcast-align                          Catch unsafe pointer casts
  # -Wmissing-declarations                Catch defined functions without prior declaration
  # -Wno-error=extra                      Downgrade some -Wextra warnings from errors to warnings
  # -Wno-error=delete-non-virtual-dtor    <-- see if still needed?
  # -Wno-error=sign-compare               Downgrade comparing signed vs. unsigned integers from errors to warnings
  # -Wno-error=missing-declarations       Downgrade -Wmissing-declarations from errors to warnings
  # -Wno-error=sfinae-incomplete          GCC 16+: Qt MOC + protobuf forward declarations trigger this
  set(ADDITIONAL_DEBUG_FLAGS
      -Wcast-align
      -Wmissing-declarations
      -Wno-error=extra
      -Wno-error=delete-non-virtual-dtor
      -Wno-error=sign-compare
      -Wno-error=missing-declarations
      -Wno-error=sfinae-incomplete
  )

  foreach(FLAG ${ADDITIONAL_DEBUG_FLAGS})
    check_cxx_compiler_flag("${FLAG}" CXX_HAS_WARNING_${FLAG})
    if(CXX_HAS_WARNING_${FLAG})
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${FLAG}")
    endif()
  endforeach()
else() # Other: macOS/LLVM, BSD/LLVM, "Clang-specific"
  # -O2                      Balanced optimization
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")

  # -g                       Include debug information (equivalent to -ggdb for GCC)
  # -O0                      No optimization
  # -Wall                    Enable all warnings
  # -Wextra                  Enable extra warnings
  # -Werror                  Treat warnings as compiler errors
  # -Wno-unused-parameter    Suppress warnings about unused function parameters (common in Qt callbacks)
  if(WARNING_AS_ERROR)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Werror -Wno-unused-parameter")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
  endif()
endif()

# GNU systems need to define the Mersenne Exponent for SFMT for the RNG to compile without warning
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions("-DSFMT_MEXP=19937")
endif()

find_package(Threads REQUIRED)

# Determine 32 or 64 bit build
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_lib_suffix 64) # 64bit
else()
  set(_lib_suffix 32) # 32bit
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

# Find OpenSSL
if(WIN32) # Windows (including 64bit)
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

# Find Visual C++ Redistributable
if(MSVC) # MS Visual C++ compiler
  find_package(VCredistRuntime)
endif()

# Package builder
set(CPACK_PACKAGE_CONTACT "Zach Halpern <zach@cockatrice.us>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Cockatrice Development Team")
set(CPACK_PACKAGE_DESCRIPTION
    "Cockatrice is an open-source, multiplatform application for playing tabletop card games over a network. \
    The program's server design prevents users from manipulating the game for unfair advantage. \
    The client also provides a single-player mode, which allows users to brew while offline."
)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_VERSION_FILENAME}")

if(UNIX)
  if(APPLE) # macOS
    set(CPACK_GENERATOR DragNDrop ${CPACK_GENERATOR})
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(CPACK_SYSTEM_NAME "OSX")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice/resources/appicon.icns")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeDMGSetup.script")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dmgBackground.tif")
    set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SignMacApplications.cmake")
  else() # Linux
    if(CPACK_GENERATOR STREQUAL "RPM")
      set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
      set(CPACK_RPM_MAIN_COMPONENT "cockatrice")
      set(CPACK_RPM_PACKAGE_REQUIRES "protobuf, qt6-qttools, qt6-qtsvg, qt6-qtmultimedia, qt6-qtimageformats")
      set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
      set(CPACK_RPM_PACKAGE_URL "http://github.com/Cockatrice/Cockatrice")
      # Stop directories from creating package conflicts
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
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6multimedia6, libqt6svg6, qt6-qpa-plugins, qt6-image-formats-plugins")
      set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "libqt6sql6-mysql") # for connecting Servatrice to a MySQL DB
    endif()
  endif()
elseif(WIN32) # Windows (including 64bit)
  set(CPACK_GENERATOR NSIS ${CPACK_GENERATOR})
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TRICE_IS_64_BIT 1) # 64bit
  else()
    set(TRICE_IS_64_BIT 0) # 32bit
  endif()

  # Configure file with custom definitions for NSIS
  configure_file("${COCKATRICE_CMAKE_PATH}/NSIS.definitions.nsh.in" "${PROJECT_BINARY_DIR}/NSIS.definitions.nsh")

  # Include vcredist into the package - NSIS will take care of running it
  if(VCREDISTRUNTIME_FOUND)
    install(FILES "${VCREDISTRUNTIME_FILE}" DESTINATION ./)
  endif()
endif()

include(CPack)

add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_card ${CMAKE_BINARY_DIR}/libcockatrice_card)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_deck_list ${CMAKE_BINARY_DIR}/libcockatrice_deck_list)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_interfaces ${CMAKE_BINARY_DIR}/libcockatrice_interfaces)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_protocol ${CMAKE_BINARY_DIR}/libcockatrice_protocol)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_rng ${CMAKE_BINARY_DIR}/libcockatrice_rng)
add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_utility ${CMAKE_BINARY_DIR}/libcockatrice_utility)

if(WITH_CLIENT
   OR WITH_ORACLE
   OR WITH_SERVER
)
  add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_network ${CMAKE_BINARY_DIR}/libcockatrice_network)
endif()

if(WITH_CLIENT OR WITH_ORACLE)
  add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_filters ${CMAKE_BINARY_DIR}/libcockatrice_filters)
  add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_models ${CMAKE_BINARY_DIR}/libcockatrice_models)
  add_subdirectory(${CMAKE_SOURCE_DIR}/libcockatrice_settings ${CMAKE_BINARY_DIR}/libcockatrice_settings)
endif()

if(WITH_CLIENT)
  add_subdirectory(cockatrice)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Cockatrice;Cockatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(WITH_ORACLE)
  add_subdirectory(oracle)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Oracle;Oracle;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(WITH_SERVER)
  add_subdirectory(servatrice)
  set(CPACK_INSTALL_CMAKE_PROJECTS "Servatrice;Servatrice;ALL;/" ${CPACK_INSTALL_CMAKE_PROJECTS})
endif()

if(TEST)
  include(CTest)
  add_subdirectory(tests)
endif()

if(Qt6_FOUND AND Qt6_VERSION_MINOR GREATER_EQUAL 3)
  # Qt 6.3+ requires project finalization to support translations
  qt6_finalize_project()
endif()
