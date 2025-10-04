set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    include/deck_list/abstract_deck_list_card_node.h
    include/deck_list/abstract_deck_list_node.h
    include/deck_list/deck_list.h
    include/deck_list/deck_list_card_node.h
    include/deck_list/inner_deck_list_node.h
)

if (Qt6_FOUND)
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif (Qt5_FOUND)
    qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif ()

qt_add_library(cockatrice_deck_list STATIC
    src/abstract_deck_list_card_node.cpp
    src/abstract_deck_list_node.cpp
    src/deck_list.cpp
    src/deck_list_card_node.cpp
    src/inner_deck_list_node.cpp
    ${MOC_SOURCES}
)

target_include_directories(cockatrice_deck_list
    PUBLIC include
    PUBLIC ${CMAKE_SOURCE_DIR}/libs/utility/include
)

target_link_libraries(cockatrice_deck_list
    PUBLIC utility ${COCKATRICE_QT_MODULES}
)
