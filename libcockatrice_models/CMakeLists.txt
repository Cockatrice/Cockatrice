set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/models/database/card_database_model.h
    libcockatrice/models/database/card_database_display_model.h
    libcockatrice/models/database/card/card_completer_proxy_model.h
    libcockatrice/models/database/card/card_search_model.h
    libcockatrice/models/database/card_set/card_sets_model.h
    libcockatrice/models/database/token/token_display_model.h
    libcockatrice/models/database/token/token_edit_model.h
    libcockatrice/models/deck_list/deck_list_model.h
    libcockatrice/models/deck_list/deck_list_sort_filter_proxy_model.h
    libcockatrice/models/deck_list/deck_loader.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_models STATIC
  ${MOC_SOURCES}
  libcockatrice/models/database/card_database_model.cpp
  libcockatrice/models/database/card_database_display_model.cpp
  libcockatrice/models/database/card/card_completer_proxy_model.cpp
  libcockatrice/models/database/card/card_search_model.cpp
  libcockatrice/models/database/card_set/card_sets_model.cpp
  libcockatrice/models/database/token/token_display_model.cpp
  libcockatrice/models/database/token/token_edit_model.cpp
  libcockatrice/models/deck_list/deck_list_model.cpp
  libcockatrice/models/deck_list/deck_list_sort_filter_proxy_model.cpp
  libcockatrice/models/deck_list/deck_loader.cpp
)

target_include_directories(libcockatrice_models PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_models PUBLIC libcockatrice_card libcockatrice_deck_list ${COCKATRICE_QT_MODULES})
