# Find a compatible Qt version
# Inputs: WITH_SERVER, WITH_CLIENT, WITH_ORACLE
# Optional Input: QT6_DIR -- Hint as to where Qt6 lives on the system
# Output: COCKATRICE_QT_VERSION_NAME -- Example values: Qt6
# Output: SERVATRICE_QT_MODULES
# Output: COCKATRICE_QT_MODULES
# Output: ORACLE_QT_MODULES
# Output: TEST_QT_MODULES

set(REQUIRED_QT_COMPONENTS Core)

if(WITH_SERVER)
  list(APPEND REQUIRED_QT_COMPONENTS Network Sql WebSockets)
endif()
if(WITH_CLIENT)
  list(
    APPEND
    REQUIRED_QT_COMPONENTS
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
endif()
if(WITH_ORACLE)
  list(APPEND REQUIRED_QT_COMPONENTS Concurrent Network Svg Widgets)
endif()
if(TEST)
  # Union of Qt modules required across all test targets (independent of application targets).
  # When adding a new test that needs additional Qt modules, add them here rather than in the test's CMakeLists.txt
  list(APPEND REQUIRED_QT_COMPONENTS Concurrent Network Svg Widgets)
endif()

list(REMOVE_DUPLICATES REQUIRED_QT_COMPONENTS)

# Find Qt and all required components including Linguist
find_package(Qt6 REQUIRED COMPONENTS ${REQUIRED_QT_COMPONENTS} Linguist)
message(STATUS "Qt6_VERSION = ${Qt6_VERSION}")
message(STATUS "Qt6_DIR = ${Qt6_DIR}")
message(STATUS "Qt6Core_DIR = ${Qt6Core_DIR}")
message(STATUS "TEST=${TEST}")
message(STATUS "TEST_QT_MODULES=${TEST_QT_MODULES}")

set(COCKATRICE_QT_VERSION_NAME Qt6)

list(FIND Qt6LinguistTools_TARGETS Qt6::lrelease QT6_LRELEASE_INDEX)
if(QT6_LRELEASE_INDEX EQUAL -1)
  message(WARNING "Qt6 lrelease not found.")
endif()

list(FIND Qt6LinguistTools_TARGETS Qt6::lupdate QT6_LUPDATE_INDEX)
if(QT6_LUPDATE_INDEX EQUAL -1)
  message(WARNING "Qt6 lupdate not found.")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Establish Qt Plugins directory & Library directories
get_target_property(QT_LIBRARY_DIR ${COCKATRICE_QT_VERSION_NAME}::Core LOCATION)
get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} DIRECTORY)
get_filename_component(QT_PLUGINS_DIR "${Qt6Core_DIR}/../../../${QT6_INSTALL_PLUGINS}" ABSOLUTE)
get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/../../.." ABSOLUTE)
if(UNIX AND APPLE)
  # Mac needs a bit more help finding all necessary components
  list(APPEND QT_LIBRARY_DIR "/usr/local/lib")
endif()
message(DEBUG "QT_PLUGINS_DIR = ${QT_PLUGINS_DIR}")
message(DEBUG "QT_LIBRARY_DIR = ${QT_LIBRARY_DIR}")

# Establish exports
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" SERVATRICE_QT_MODULES "${_SERVATRICE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" COCKATRICE_QT_MODULES "${_COCKATRICE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" ORACLE_QT_MODULES "${_ORACLE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" TEST_QT_MODULES "${_TEST_NEEDED}")

# Core-only export (useful for headless libs)
set(QT_CORE_MODULE "${COCKATRICE_QT_VERSION_NAME}::Core")

message(STATUS "Found Qt ${${COCKATRICE_QT_VERSION_NAME}_VERSION} at: ${${COCKATRICE_QT_VERSION_NAME}_DIR}")
