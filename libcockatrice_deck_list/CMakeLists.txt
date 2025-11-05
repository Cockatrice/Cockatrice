set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/deck_list/abstract_deck_list_card_node.h libcockatrice/deck_list/abstract_deck_list_node.h
    libcockatrice/deck_list/deck_list.h libcockatrice/deck_list/deck_list_card_node.h
    libcockatrice/deck_list/inner_deck_list_node.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_deck_list STATIC
  ${MOC_SOURCES} libcockatrice/deck_list/abstract_deck_list_card_node.cpp
  libcockatrice/deck_list/abstract_deck_list_node.cpp libcockatrice/deck_list/deck_list.cpp
  libcockatrice/deck_list/deck_list_card_node.cpp libcockatrice/deck_list/inner_deck_list_node.cpp
)

add_dependencies(libcockatrice_deck_list libcockatrice_protocol)

target_include_directories(libcockatrice_deck_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_deck_list PUBLIC libcockatrice_protocol libcockatrice_utility ${QT_CORE_MODULE})
