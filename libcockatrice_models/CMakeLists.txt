set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(libcockatrice/models/database)
add_subdirectory(libcockatrice/models/deck_list)

add_library(libcockatrice_models INTERFACE)

target_include_directories(libcockatrice_models INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_models INTERFACE libcockatrice_models_database libcockatrice_models_deck_list)
