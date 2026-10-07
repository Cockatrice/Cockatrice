set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/game/abstract_game.h
    libcockatrice/game/board/arrow_data.h
    libcockatrice/game/board/card_list.h
    libcockatrice/game/board/card_state.h
    libcockatrice/game/board/counter_state.h
    libcockatrice/game/game.h
    libcockatrice/game/game_event_handler.h
    libcockatrice/game/game_meta_info.h
    libcockatrice/game/game_state.h
    libcockatrice/game/phase.h
    libcockatrice/game/player/card_menu_action_type.h
    libcockatrice/game/player/card_move_request.h
    libcockatrice/game/player/event_processing_options.h
    libcockatrice/game/player/move_top_cards_until_options.h
    libcockatrice/game/player/player_actions.h
    libcockatrice/game/player/player_event_handler.h
    libcockatrice/game/player/player_info.h
    libcockatrice/game/player/player_logic.h
    libcockatrice/game/player/player_manager.h
    libcockatrice/game/player/token_info.h
    libcockatrice/game/replay.h
    libcockatrice/game/translation.h
    libcockatrice/game/zones/card_zone_algorithms.h
    libcockatrice/game/zones/card_zone_logic.h
    libcockatrice/game/zones/hand_zone_logic.h
    libcockatrice/game/zones/pile_zone_logic.h
    libcockatrice/game/zones/stack_zone_logic.h
    libcockatrice/game/zones/table_zone_logic.h
    libcockatrice/game/zones/view_zone_logic.h
)

qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

add_library(
  libcockatrice_game STATIC
  ${MOC_SOURCES}
  libcockatrice/game/abstract_game.cpp
  libcockatrice/game/board/arrow_data.cpp
  libcockatrice/game/board/card_list.cpp
  libcockatrice/game/board/card_state.cpp
  libcockatrice/game/board/counter_state.cpp
  libcockatrice/game/game.cpp
  libcockatrice/game/game_event_handler.cpp
  libcockatrice/game/game_meta_info.cpp
  libcockatrice/game/game_state.cpp
  libcockatrice/game/phase.cpp
  libcockatrice/game/player/player_actions.cpp
  libcockatrice/game/player/player_event_handler.cpp
  libcockatrice/game/player/player_info.cpp
  libcockatrice/game/player/player_logic.cpp
  libcockatrice/game/player/player_manager.cpp
  libcockatrice/game/replay.cpp
  libcockatrice/game/zones/card_zone_logic.cpp
  libcockatrice/game/zones/hand_zone_logic.cpp
  libcockatrice/game/zones/pile_zone_logic.cpp
  libcockatrice/game/zones/stack_zone_logic.cpp
  libcockatrice/game/zones/table_zone_logic.cpp
  libcockatrice/game/zones/view_zone_logic.cpp
)

target_include_directories(
  libcockatrice_game
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # SettingsCache, the client-side settings facade the game logic still reads, lives in the
  # application sources rather than in libcockatrice_settings.
  PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src
)

target_link_libraries(
  libcockatrice_game
  PUBLIC libcockatrice_card
         libcockatrice_deck_list
         libcockatrice_filters
         libcockatrice_network
         libcockatrice_protocol
         libcockatrice_settings
         libcockatrice_utility
         ${QT_CORE_MODULE}
         ${COCKATRICE_QT_VERSION_NAME}::Concurrent
         ${COCKATRICE_QT_VERSION_NAME}::Gui
         ${COCKATRICE_QT_VERSION_NAME}::Widgets
)
