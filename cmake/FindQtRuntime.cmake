# FindQtRuntime.cmake
#
# Find a compatible Qt version
#
# Inputs:
#   WITH_CLIENT
#   WITH_ORACLE
#   WITH_SERVER
#   TEST
#
# Outputs:
#   QT_MAIN_VERSION_STRING
#   QT_CORE_MODULE
#   QT_MODULES_COCKATRICE
#   QT_MODULES_ORACLE
#   QT_MODULES_SERVATRICE
#   QT_MODULES_TEST
#   QT_LIBRARY_DIR
#   QT_PLUGINS_DIR

# ---------------------------------------------------------------------------
# Define the Qt components required by each target
# ---------------------------------------------------------------------------

set(QT_COMPONENTS_COCKATRICE
    Concurrent
    Gui
    Multimedia
    Network
    PrintSupport
    ShaderTools
    Svg
    WebSockets
    Widgets
    Xml
    Quick
    QuickWidgets
)

set(QT_COMPONENTS_ORACLE Concurrent Network Svg Widgets)

set(QT_COMPONENTS_SERVATRICE Network Sql WebSockets)

# Union of Qt modules required across all test targets (independent of application targets).
# When adding a new test that needs additional Qt modules, add them here instead to tests/CMakeLists.txt.
set(QT_COMPONENTS_TEST Concurrent Network Svg Widgets)

# ---------------------------------------------------------------------------
# Determine which Qt components are required for this build
# ---------------------------------------------------------------------------

set(QT_COMPONENTS_REQUIRED Core)

if(WITH_CLIENT)
  list(APPEND QT_COMPONENTS_REQUIRED ${QT_COMPONENTS_COCKATRICE})
endif()

if(WITH_ORACLE)
  list(APPEND QT_COMPONENTS_REQUIRED ${QT_COMPONENTS_ORACLE})
endif()

if(WITH_SERVER)
  list(APPEND QT_COMPONENTS_REQUIRED ${QT_COMPONENTS_SERVATRICE})
endif()

if(TEST)
  list(APPEND QT_COMPONENTS_REQUIRED ${QT_COMPONENTS_TEST})
endif()

list(REMOVE_DUPLICATES QT_COMPONENTS_REQUIRED)

# ---------------------------------------------------------------------------
# Find Qt and define minimum version centrally
# ---------------------------------------------------------------------------

# Add Qt Linguist as required component unrelated of build target
find_package(Qt6 6.4 REQUIRED COMPONENTS ${QT_COMPONENTS_REQUIRED} LinguistTools)

set(QT_MAIN_VERSION_STRING Qt6)

# ---------------------------------------------------------------------------
# Qt Linguist tools
# ---------------------------------------------------------------------------

if(TARGET Qt6::lrelease)
  set(QT6_LRELEASE_INDEX 0)
else()
  message(WARNING "Qt6 lrelease not found.")
endif()

if(TARGET Qt6::lupdate)
  set(QT6_LUPDATE_INDEX 0)
else()
  message(WARNING "Qt6 lupdate not found.")
endif()

# ---------------------------------------------------------------------------
# Export Qt target lists for individual targets
# ---------------------------------------------------------------------------

# Convert components list into Qt modules, e.g.
# Network;Sql;WebSockets --> Qt6::Network;Qt6::Sql;Qt6::WebSockets
function(_qt_components_to_targets COMPONENTS OUTPUT_VARIABLE)
  set(TARGETS)

  foreach(COMPONENT IN LISTS COMPONENTS)
    list(APPEND TARGETS "${QT_MAIN_VERSION_STRING}::${COMPONENT}")
  endforeach()

  set(${OUTPUT_VARIABLE}
      "${TARGETS}"
      PARENT_SCOPE
  )
endfunction()

if(WITH_CLIENT)
  _qt_components_to_targets("${QT_COMPONENTS_COCKATRICE}" QT_MODULES_COCKATRICE)
endif()

if(WITH_ORACLE)
  _qt_components_to_targets("${QT_COMPONENTS_ORACLE}" QT_MODULES_ORACLE)
endif()

if(WITH_SERVER)
  _qt_components_to_targets("${QT_COMPONENTS_SERVATRICE}" QT_MODULES_SERVATRICE)
endif()

if(TEST)
  _qt_components_to_targets("${QT_COMPONENTS_TEST}" QT_MODULES_TEST)
endif()

# Core-only export (useful for headless libraries)
set(QT_CORE_MODULE "${QT_MAIN_VERSION_STRING}::Core")

# ---------------------------------------------------------------------------
# Qt runtime library & plugin paths
# ---------------------------------------------------------------------------

if(NOT TARGET "${QT_CORE_MODULE}")
  message(FATAL_ERROR "${QT_CORE_MODULE} target is not available")
endif()

get_target_property(QT_LIBRARY_DIR "${QT_CORE_MODULE}" LOCATION)
get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}" DIRECTORY)
get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/../../.." ABSOLUTE)
get_filename_component(QT_PLUGINS_DIR "${Qt6Core_DIR}/../../../${QT6_INSTALL_PLUGINS}" ABSOLUTE)

if(UNIX AND APPLE)
  # macOS needs a bit more help finding all necessary components
  list(APPEND QT_LIBRARY_DIR "/usr/local/lib")
endif()

# ---------------------------------------------------------------------------
# Debug information
# ---------------------------------------------------------------------------

message(STATUS "Found Qt: ${Qt6_DIR} (found version \"${Qt6_VERSION}\")")
message(STATUS "QT_COMPONENTS_REQUIRED = ${QT_COMPONENTS_REQUIRED}")
if(WITH_CLIENT)
  message(STATUS "QT_MODULES_COCKATRICE = ${QT_MODULES_COCKATRICE}")
endif()
if(WITH_ORACLE)
  message(STATUS "QT_MODULES_ORACLE = ${QT_MODULES_ORACLE}")
endif()
if(WITH_SERVER)
  message(STATUS "QT_MODULES_SERVATRICE = ${QT_MODULES_SERVATRICE}")
endif()
if(TEST)
  message(STATUS "QT_MODULES_TEST = ${QT_MODULES_TEST}")
endif()

message(DEBUG "QT_PLUGINS_DIR = ${QT_PLUGINS_DIR}")
message(DEBUG "QT_LIBRARY_DIR = ${QT_LIBRARY_DIR}")
