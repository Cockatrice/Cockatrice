set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS deck_list_model.h deck_list_sort_filter_proxy_model.h deck_loader.h)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(
  libcockatrice_models_deck_list STATIC ${MOC_SOURCES} deck_list_model.cpp deck_list_sort_filter_proxy_model.cpp
                                        deck_loader.cpp
)

target_include_directories(libcockatrice_models_deck_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  libcockatrice_models_deck_list PUBLIC libcockatrice_card libcockatrice_deck_list ${COCKATRICE_QT_MODULES}
)
