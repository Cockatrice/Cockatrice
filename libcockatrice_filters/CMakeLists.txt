set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS libcockatrice/filters/filter_card.h libcockatrice/filters/filter_string.h
            libcockatrice/filters/filter_tree.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_filters STATIC ${MOC_SOURCES} libcockatrice/filters/filter_card.cpp
                               libcockatrice/filters/filter_string.cpp libcockatrice/filters/filter_tree.cpp
)

add_dependencies(libcockatrice_filters libcockatrice_card)

target_include_directories(libcockatrice_filters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_filters PUBLIC libcockatrice_card ${QT_CORE_MODULE})
