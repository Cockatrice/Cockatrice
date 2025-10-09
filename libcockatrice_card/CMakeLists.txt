set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/card/card_info.h
    libcockatrice/card/card_info_comparator.h
    libcockatrice/card/database/card_database.h
    libcockatrice/card/database/card_database_loader.h
    libcockatrice/card/database/card_database_manager.h
    libcockatrice/card/database/card_database_querier.h
    libcockatrice/card/database/model/card_database_model.h
    libcockatrice/card/database/model/card_database_display_model.h
    libcockatrice/card/database/model/card/card_completer_proxy_model.h
    libcockatrice/card/database/model/card/card_search_model.h
    libcockatrice/card/database/model/card_set/card_sets_model.h
    libcockatrice/card/database/model/token/token_display_model.h
    libcockatrice/card/database/model/token/token_edit_model.h
    libcockatrice/card/database/parser/card_database_parser.h
    libcockatrice/card/database/parser/cockatrice_xml_3.h
    libcockatrice/card/database/parser/cockatrice_xml_4.h
    libcockatrice/card/printing/exact_card.h
    libcockatrice/card/printing/printing_info.h
    libcockatrice/card/set/card_set.h
    libcockatrice/card/relation/card_relation.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_card STATIC
  ${MOC_SOURCES}
  libcockatrice/card/card_info.cpp
  libcockatrice/card/card_info_comparator.cpp
  libcockatrice/card/database/card_database.cpp
  libcockatrice/card/database/card_database_loader.cpp
  libcockatrice/card/database/card_database_manager.cpp
  libcockatrice/card/database/card_database_querier.cpp
  libcockatrice/card/database/model/card/card_completer_proxy_model.cpp
  libcockatrice/card/database/model/card/card_search_model.cpp
  libcockatrice/card/database/model/card_set/card_sets_model.cpp
  libcockatrice/card/database/model/card_database_display_model.cpp
  libcockatrice/card/database/model/card_database_model.cpp
  libcockatrice/card/database/model/token/token_display_model.cpp
  libcockatrice/card/database/model/token/token_edit_model.cpp
  libcockatrice/card/database/parser/card_database_parser.cpp
  libcockatrice/card/database/parser/cockatrice_xml_3.cpp
  libcockatrice/card/database/parser/cockatrice_xml_4.cpp
  libcockatrice/card/printing/exact_card.cpp
  libcockatrice/card/printing/printing_info.cpp
  libcockatrice/card/relation/card_relation.cpp
  libcockatrice/card/set/card_set.cpp
  libcockatrice/card/set/card_set_list.cpp
)

target_include_directories(
  libcockatrice_card
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_SOURCE_DIR}/cockatrice/src/filters
)

target_link_libraries(
  libcockatrice_card
  PUBLIC libcockatrice_settings
  PUBLIC ${COCKATRICE_QT_MODULES}
)
