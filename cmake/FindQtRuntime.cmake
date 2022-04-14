# Find a compatible Qt version
# Sets COCKATRICE_QT_VERSION_NAME -- Example values: Qt5, Qt6

FIND_PACKAGE(Qt6 6.2.3 COMPONENTS
        Concurrent
        Core5Compat
        Gui
        LinguistTools
        Multimedia
        Network
        PrintSupport
        Sql
        Svg
        WebSockets
        Widgets
        QUIET
        HINTS ${Qt6_DIR}
)

FIND_PACKAGE(Qt5 5.8.0 COMPONENTS
        Concurrent
        LinguistTools
        Multimedia
        Network
        PrintSupport
        Sql
        Svg
        WebSockets
        Widgets
        QUIET
        HINTS ${Qt5_DIR}
)

IF(Qt6_FOUND AND NOT FORCE_USE_QT5)
    SET(COCKATRICE_QT_VERSION_NAME Qt6)
    UNSET(Qt5_FOUND)
ELSEIF(Qt5_FOUND)
    SET(COCKATRICE_QT_VERSION_NAME Qt5)
    UNSET(Qt6_FOUND)
ELSE()
    MESSAGE(FATAL_ERROR "No suitable version of Qt was found")
ENDIF()

IF(${COCKATRICE_QT_VERSION_NAME}_POSITION_INDEPENDENT_CODE OR Qt6_FOUND)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()

# guess plugins and libraries directory
SET(QT_PLUGINS_DIR "${${COCKATRICE_QT_VERSION_NAME}Core_DIR}/../../../plugins")
GET_TARGET_PROPERTY(QT_LIBRARY_DIR ${COCKATRICE_QT_VERSION_NAME}::Core LOCATION)
GET_FILENAME_COMPONENT(QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH)

MESSAGE(STATUS "Found Qt ${${COCKATRICE_QT_VERSION_NAME}_VERSION} at: ${${COCKATRICE_QT_VERSION_NAME}_DIR}")
