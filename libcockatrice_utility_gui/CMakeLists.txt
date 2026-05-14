cmake_minimum_required(VERSION 3.16)
project(UtilityGui VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Sibling of libcockatrice_utility but carries the Qt::Gui dependency so that
# libcockatrice_utility (consumed transitively by the headless servatrice) can
# stay Core+Network only.  Host Gui-needing shared utility code here.

set(UTILITY_GUI_HEADERS libcockatrice/utility_gui/url_scheme_event_filter.h)

# Header-only Q_OBJECT classes need a .cpp anchor so AUTOMOC has somewhere to
# compile the generated meta-object code.  An INTERFACE library skips AUTOMOC,
# so we use a STATIC lib + tiny stub.
add_library(libcockatrice_utility_gui STATIC ${UTILITY_GUI_HEADERS} libcockatrice/utility_gui/stub.cpp)

target_include_directories(libcockatrice_utility_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_utility_gui PUBLIC ${QT_CORE_MODULE} ${QT_GUI_MODULE})
