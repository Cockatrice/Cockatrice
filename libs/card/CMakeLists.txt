set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    include/card/card_info.h
    include/card/card_info_comparator.h
    include/card/card_database/card_database.h
    include/card/card_database/card_database_loader.h
    include/card/card_database/card_database_manager.h
    include/card/card_database/card_database_querier.h
    include/card/card_database/model/card_database_model.h
    include/card/card_database/model/card_database_display_model.h
    include/card/card_database/model/card/card_completer_proxy_model.h
    include/card/card_database/model/card/card_search_model.h
    include/card/card_database/model/token/token_display_model.h
    include/card/card_database/model/token/token_edit_model.h
    include/card/card_database/parser/card_database_parser.h
    include/card/card_database/parser/cockatrice_xml_3.h
    include/card/card_database/parser/cockatrice_xml_4.h
    include/card/card_printing/exact_card.h
    include/card/card_printing/printing_info.h
    include/card/card_set/card_set.h
    include/card/card_relation/card_relation.h
)

if (Qt6_FOUND)
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif (Qt5_FOUND)
    qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif ()

qt_add_library(cardlib STATIC
    src/card_info.cpp
    src/card_info_comparator.cpp
    src/card_database/card_database.cpp
    src/card_database/card_database_loader.cpp
    src/card_database/card_database_manager.cpp
    src/card_database/card_database_querier.cpp
    src/card_database/model/card_database_model.cpp
    src/card_database/model/card_database_display_model.cpp
    src/card_database/model/card/card_completer_proxy_model.cpp
    src/card_database/model/card/card_search_model.cpp
    src/card_database/model/token/token_display_model.cpp
    src/card_database/model/token/token_edit_model.cpp
    src/card_database/parser/card_database_parser.cpp
    src/card_database/parser/cockatrice_xml_3.cpp
    src/card_database/parser/cockatrice_xml_4.cpp
    src/card_printing/exact_card.cpp
    src/card_printing/printing_info.cpp
    src/card_set/card_set.cpp
    src/card_set/card_set_list.cpp
    src/card_relation/card_relation.cpp
    ${MOC_SOURCES}
)

target_include_directories(cardlib
    PUBLIC include
    PUBLIC ${CMAKE_SOURCE_DIR}/common
    PUBLIC ${CMAKE_SOURCE_DIR}/libs/settings/include
    PUBLIC ${CMAKE_SOURCE_DIR}/cockatrice/src/filters
)

target_link_libraries(cardlib
    PUBLIC settingslib
    PUBLIC ${COCKATRICE_QT_MODULES}
)
