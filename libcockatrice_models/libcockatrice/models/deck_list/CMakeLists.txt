set(HEADERS deck_list_model.h deck_list_sort_filter_proxy_model.h)

add_library(
  libcockatrice_models_deck_list STATIC ${HEADERS} deck_list_model.cpp deck_list_model_custom_zones.cpp
                                        deck_list_sort_filter_proxy_model.cpp
)

target_include_directories(libcockatrice_models_deck_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
  libcockatrice_models_deck_list PUBLIC libcockatrice_card libcockatrice_deck_list ${QT_CORE_MODULE}
)

target_precompile_headers(libcockatrice_models_deck_list PRIVATE "${CMAKE_SOURCE_DIR}/cmake/pch/qtcore_pch.h")
