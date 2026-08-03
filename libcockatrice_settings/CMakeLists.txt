set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/settings/cache_storage_settings.h
    libcockatrice/settings/card_database_settings.h
    libcockatrice/settings/card_override_settings.h
    libcockatrice/settings/cards_display_settings.h
    libcockatrice/settings/chat_settings.h
    libcockatrice/settings/debug_settings.h
    libcockatrice/settings/download_settings.h
    libcockatrice/settings/game_filters_settings.h
    libcockatrice/settings/game_settings.h
    libcockatrice/settings/interface_settings.h
    libcockatrice/settings/layouts_settings.h
    libcockatrice/settings/message_settings.h
    libcockatrice/settings/paths_settings.h
    libcockatrice/settings/personal_settings.h
    libcockatrice/settings/recents_settings.h
    libcockatrice/settings/servers_settings.h
    libcockatrice/settings/settings_manager.h
    libcockatrice/settings/settings_migration.h
    libcockatrice/settings/sound_settings.h
    libcockatrice/settings/tabs_settings.h
    libcockatrice/settings/updates_settings.h
    libcockatrice/settings/visual_deck_storage_settings.h
)

qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

add_library(
  libcockatrice_settings STATIC
  ${MOC_SOURCES}
  libcockatrice/settings/cache_storage_settings.cpp
  libcockatrice/settings/card_database_settings.cpp
  libcockatrice/settings/card_override_settings.cpp
  libcockatrice/settings/cards_display_settings.cpp
  libcockatrice/settings/chat_settings.cpp
  libcockatrice/settings/debug_settings.cpp
  libcockatrice/settings/download_settings.cpp
  libcockatrice/settings/game_filters_settings.cpp
  libcockatrice/settings/game_settings.cpp
  libcockatrice/settings/interface_settings.cpp
  libcockatrice/settings/layouts_settings.cpp
  libcockatrice/settings/message_settings.cpp
  libcockatrice/settings/paths_settings.cpp
  libcockatrice/settings/personal_settings.cpp
  libcockatrice/settings/recents_settings.cpp
  libcockatrice/settings/servers_settings.cpp
  libcockatrice/settings/settings_manager.cpp
  libcockatrice/settings/settings_migration.cpp
  libcockatrice/settings/sound_settings.cpp
  libcockatrice/settings/tabs_settings.cpp
  libcockatrice/settings/updates_settings.cpp
  libcockatrice/settings/visual_deck_storage_settings.cpp
)

target_include_directories(
  libcockatrice_settings
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network
)

target_link_libraries(libcockatrice_settings PUBLIC libcockatrice_card libcockatrice_utility ${QT_CORE_MODULE})
