# Find a compatible Qt version
# Inputs: WITH_SERVER, WITH_CLIENT, WITH_ORACLE, WITH_DBCONVERTER, FORCE_USE_QT5
# Outputs: COCKATRICE_QT_VERSION_NAME -- Example values: Qt5, Qt6

set(COCKATRICE_QT_COMPONENTS Core Core5Compat)
if(WITH_SERVER)
    set(COCKATRICE_QT_COMPONENTS ${COCKATRICE_QT_COMPONENTS} Network Sql WebSockets)
endif()
if(WITH_CLIENT)
    set(COCKATRICE_QT_COMPONENTS ${COCKATRICE_QT_COMPONENTS} Concurrent Multimedia Network PrintSupport Svg Widgets WebSockets)
endif()
if(WITH_ORACLE)
    set(COCKATRICE_QT_COMPONENTS ${COCKATRICE_QT_COMPONENTS} Concurrent Network Svg Widgets)
endif()
if(WITH_DBCONVERTER)
    set(COCKATRICE_QT_COMPONENTS ${COCKATRICE_QT_COMPONENTS} Network Widgets)
endif()
list(REMOVE_DUPLICATES COCKATRICE_QT_COMPONENTS)

if(NOT FORCE_USE_QT5)
    find_package(Qt6 6.2.3 COMPONENTS ${COCKATRICE_QT_COMPONENTS} QUIET HINTS ${Qt6_DIR})
endif()
if(Qt6_FOUND)
    set(COCKATRICE_QT_VERSION_NAME Qt6)
else()
    list(REMOVE_ITEM COCKATRICE_QT_COMPONENTS Core5Compat) # Core5Compat is Qt6 Only
    find_package(Qt5 5.8.0 COMPONENTS ${COCKATRICE_QT_COMPONENTS} QUIET HINTS ${Qt5_DIR})
    if(Qt5_FOUND)
        set(COCKATRICE_QT_VERSION_NAME Qt5)
    else()
        message(FATAL_ERROR "No suitable version of Qt was found")
    endif()
endif()

if(Qt5_POSITION_INDEPENDENT_CODE OR Qt6_FOUND)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# guess plugins and libraries directory
set(QT_PLUGINS_DIR "${${COCKATRICE_QT_VERSION_NAME}Core_DIR}/../../../plugins")
get_target_property(QT_LIBRARY_DIR ${COCKATRICE_QT_VERSION_NAME}::Core LOCATION)
get_filename_component(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)

message(STATUS "Found Qt ${${COCKATRICE_QT_VERSION_NAME}_VERSION} at: ${${COCKATRICE_QT_VERSION_NAME}_DIR}")
