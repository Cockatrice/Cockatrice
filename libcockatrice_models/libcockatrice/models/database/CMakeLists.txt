set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    card_database_model.h
    card_database_display_model.h
    card/card_completer_proxy_model.h
    card/card_search_model.h
    card_set/card_sets_model.h
    token/token_display_model.h
    token/token_edit_model.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_models_database STATIC
  ${MOC_SOURCES}
  card_database_model.cpp
  card_database_display_model.cpp
  card/card_completer_proxy_model.cpp
  card/card_search_model.cpp
  card_set/card_sets_model.cpp
  token/token_display_model.cpp
  token/token_edit_model.cpp
)

target_include_directories(libcockatrice_models_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_models_database PUBLIC libcockatrice_card libcockatrice_filters ${QT_CORE_MODULE})
