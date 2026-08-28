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
#   COCKATRICE_QT_VERSION_NAME
#   QT_CORE_MODULE
#   COCKATRICE_QT_MODULES
#   ORACLE_QT_MODULES
#   SERVATRICE_QT_MODULES
#   TEST_QT_MODULES
#   QT_LIBRARY_DIR
#   QT_PLUGINS_DIR

set(COCKATRICE_QT_VERSION_NAME Qt6)

# ---------------------------------------------------------------------------
# Define the Qt components required by each target
# ---------------------------------------------------------------------------

set(COCKATRICE_QT_COMPONENTS
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

set(ORACLE_QT_COMPONENTS Concurrent Network Svg Widgets)

set(SERVATRICE_QT_COMPONENTS Network Sql WebSockets)

set(TEST_QT_COMPONENTS Concurrent Network Svg Widgets)

# ---------------------------------------------------------------------------
# Determine which Qt components are required for this build
# ---------------------------------------------------------------------------

set(REQUIRED_QT_COMPONENTS Core)

if(WITH_CLIENT)
  list(APPEND REQUIRED_QT_COMPONENTS ${COCKATRICE_QT_COMPONENTS})
endif()

if(WITH_ORACLE)
  list(APPEND REQUIRED_QT_COMPONENTS ${ORACLE_QT_COMPONENTS})
endif()

if(WITH_SERVER)
  list(APPEND REQUIRED_QT_COMPONENTS ${SERVATRICE_QT_COMPONENTS})
endif()

if(TEST)
  list(APPEND REQUIRED_QT_COMPONENTS ${TEST_QT_COMPONENTS})
endif()

list(REMOVE_DUPLICATES REQUIRED_QT_COMPONENTS)

# ---------------------------------------------------------------------------
# Find Qt and define minimum version centrally
# ---------------------------------------------------------------------------

find_package(Qt6 6.4 REQUIRED COMPONENTS ${REQUIRED_QT_COMPONENTS} LinguistTools)

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
# Convert the component list such as:
#   Network;Sql;WebSockets
# into Qt modules:
#   Qt6::Network;Qt6::Sql;Qt6::WebSockets
# ---------------------------------------------------------------------------

function(_qt_components_to_targets COMPONENTS OUTPUT_VARIABLE)
  set(TARGETS)

  foreach(COMPONENT IN LISTS COMPONENTS)
    list(APPEND TARGETS "${COCKATRICE_QT_VERSION_NAME}::${COMPONENT}")
  endforeach()

  set(${OUTPUT_VARIABLE}
      "${TARGETS}"
      PARENT_SCOPE
  )
endfunction()

# ---------------------------------------------------------------------------
# Export Qt target lists for the individual targets
# ---------------------------------------------------------------------------

if(WITH_CLIENT)
  _qt_components_to_targets("${COCKATRICE_QT_COMPONENTS}" COCKATRICE_QT_MODULES)
endif()

if(WITH_ORACLE)
  _qt_components_to_targets("${ORACLE_QT_COMPONENTS}" ORACLE_QT_MODULES)
endif()

if(WITH_SERVER)
  _qt_components_to_targets("${SERVATRICE_QT_COMPONENTS}" SERVATRICE_QT_MODULES)
endif()

if(TEST)
  _qt_components_to_targets("${TEST_QT_COMPONENTS}" TEST_QT_MODULES)
endif()

# Core-only export (useful for headless libraries)
set(QT_CORE_MODULE "${COCKATRICE_QT_VERSION_NAME}::Core")

# ---------------------------------------------------------------------------
# Qt runtime/plugin paths
# ---------------------------------------------------------------------------

if(NOT TARGET "${QT_CORE_MODULE}")
  message(FATAL_ERROR "${QT_CORE_MODULE} target is not available")
endif()

get_target_property(QT_LIBRARY_DIR "${QT_CORE_MODULE}" LOCATION)
get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}" DIRECTORY)
get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/../../.." ABSOLUTE)
get_filename_component(QT_PLUGINS_DIR "${Qt6Core_DIR}/../../../${QT6_INSTALL_PLUGINS}" ABSOLUTE)

if(UNIX AND APPLE)
  # macOS needs a bit more help finding all necessary components.
  list(APPEND QT_LIBRARY_DIR "/usr/local/lib")
endif()

# ---------------------------------------------------------------------------
# Debug information
# ---------------------------------------------------------------------------

message(STATUS "Found Qt: ${Qt6_DIR} (found version \"${Qt6_VERSION}\")")
message(STATUS "REQUIRED_QT_COMPONENTS = ${REQUIRED_QT_COMPONENTS}")
if(WITH_CLIENT)
  message(STATUS "COCKATRICE_QT_MODULES = ${COCKATRICE_QT_MODULES}")
endif()
if(WITH_ORACLE)
  message(STATUS "ORACLE_QT_MODULES = ${ORACLE_QT_MODULES}")
endif()
if(WITH_SERVER)
  message(STATUS "SERVATRICE_QT_MODULES = ${SERVATRICE_QT_MODULES}")
endif()
if(TEST)
  message(STATUS "TEST_QT_MODULES = ${TEST_QT_MODULES}")
endif()

message(DEBUG "QT_PLUGINS_DIR = ${QT_PLUGINS_DIR}")
message(DEBUG "QT_LIBRARY_DIR = ${QT_LIBRARY_DIR}")
