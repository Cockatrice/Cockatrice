# Find a compatible Qt version
# Inputs: WITH_SERVER, WITH_CLIENT, WITH_ORACLE, WITH_DBCONVERTER, FORCE_USE_QT5
# Optional Input: QT6_DIR -- Hint as to where Qt6 lives on the system
# Optional Input: QT5_DIR -- Hint as to where Qt5 lives on the system
# Output: COCKATRICE_QT_VERSION_NAME -- Example values: Qt5, Qt6
# Output: SERVATRICE_QT_MODULES
# Output: COCKATRICE_QT_MODULES
# Output: ORACLE_QT_MODULES
# Output: DBCONVERTER_QT_MODULES
# Output: TEST_QT_MODULES

set(REQUIRED_QT_COMPONENTS Core)
if(WITH_SERVER)
  set(_SERVATRICE_NEEDED Network Sql WebSockets)
endif()
if(WITH_CLIENT)
  set(_COCKATRICE_NEEDED
      Concurrent
      Gui
      Multimedia
      Network
      PrintSupport
      Svg
      Widgets
      WebSockets
  )
endif()
if(WITH_ORACLE)
  set(_ORACLE_NEEDED Concurrent Network Svg Widgets)
endif()
if(WITH_DBCONVERTER)
  set(_DBCONVERTER_NEEDED Network Widgets)
endif()
if(TEST)
  set(_TEST_NEEDED Widgets)
endif()

set(REQUIRED_QT_COMPONENTS ${REQUIRED_QT_COMPONENTS} ${_SERVATRICE_NEEDED} ${_COCKATRICE_NEEDED} ${_ORACLE_NEEDED}
                           ${_DBCONVERTER_NEEDED} ${_TEST_NEEDED}
)
list(REMOVE_DUPLICATES REQUIRED_QT_COMPONENTS)

if(NOT FORCE_USE_QT5)
  # Core5Compat is Qt6 Only, Linguist is now a component in Qt6 instead of an external package
  find_package(
    Qt6 6.2.3
    COMPONENTS Core5Compat ${REQUIRED_QT_COMPONENTS}
    OPTIONAL_COMPONENTS Linguist
    QUIET HINTS ${Qt6_DIR}
  )
endif()
if(Qt6_FOUND)
  set(COCKATRICE_QT_VERSION_NAME Qt6)

  if(Qt6LinguistTools_FOUND)
    list(FIND Qt6LinguistTools_TARGETS Qt6::lrelease QT6_LRELEASE_INDEX)
    if(QT6_LRELEASE_INDEX EQUAL -1)
      message(WARNING "Qt6 lrelease not found.")
    endif()

    list(FIND Qt6LinguistTools_TARGETS Qt6::lupdate QT6_LUPDATE_INDEX)
    if(QT6_LUPDATE_INDEX EQUAL -1)
      message(WARNING "Qt6 lupdate not found.")
    endif()
  endif()
else()
  find_package(
    Qt5 5.8.0
    COMPONENTS ${REQUIRED_QT_COMPONENTS}
    QUIET HINTS ${Qt5_DIR}
  )
  if(Qt5_FOUND)
    set(COCKATRICE_QT_VERSION_NAME Qt5)
  else()
    message(FATAL_ERROR "No suitable version of Qt was found")
  endif()

  # Qt5 Linguist is in a separate package
  find_package(Qt5LinguistTools QUIET)
  if(Qt5LinguistTools_FOUND)
    if(NOT Qt5_LRELEASE_EXECUTABLE)
      message(WARNING "Qt5 lrelease not found.")
    endif()
    if(NOT Qt5_LUPDATE_EXECUTABLE)
      message(WARNING "Qt5 lupdate not found.")
    endif()
  else()
    message(WARNING "Linguist Tools not found, cannot handle translations")
  endif()
endif()

if(Qt5_POSITION_INDEPENDENT_CODE OR Qt6_FOUND)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Establish Qt Plugins directory & Library directories
get_target_property(QT_LIBRARY_DIR ${COCKATRICE_QT_VERSION_NAME}::Core LOCATION)
get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} DIRECTORY)
if(Qt6_FOUND)
  get_filename_component(QT_PLUGINS_DIR "${Qt6Core_DIR}/../../../${QT6_INSTALL_PLUGINS}" ABSOLUTE)
  get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/../../.." ABSOLUTE)
  if(UNIX AND APPLE)
    # Mac needs a bit more help finding all necessary components
    list(APPEND QT_LIBRARY_DIR "/usr/local/lib")
  endif()
elseif(Qt5_FOUND)
  get_filename_component(QT_PLUGINS_DIR "${Qt5Core_DIR}/../../../plugins" ABSOLUTE)
  get_filename_component(QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE)
endif()
message(DEBUG "QT_PLUGINS_DIR = ${QT_PLUGINS_DIR}")
message(DEBUG "QT_LIBRARY_DIR = ${QT_LIBRARY_DIR}")

# Establish exports
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" SERVATRICE_QT_MODULES "${_SERVATRICE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" COCKATRICE_QT_MODULES "${_COCKATRICE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" ORACLE_QT_MODULES "${_ORACLE_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" DB_CONVERTER_QT_MODULES "${_DBCONVERTER_NEEDED}")
string(REGEX REPLACE "([^;]+)" "${COCKATRICE_QT_VERSION_NAME}::\\1" TEST_QT_MODULES "${_TEST_NEEDED}")
if(Qt6_FOUND)
  list(APPEND SERVATRICE_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Core5Compat)
  list(APPEND COCKATRICE_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Core5Compat)
  list(APPEND ORACLE_QT_MODULES ${COCKATRICE_QT_VERSION_NAME}::Core5Compat)
endif()

message(STATUS "Found Qt ${${COCKATRICE_QT_VERSION_NAME}_VERSION} at: ${${COCKATRICE_QT_VERSION_NAME}_DIR}")
