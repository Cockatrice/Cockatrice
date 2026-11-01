add_executable(
  game_smoke_test
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/cache_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/card_counter_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/shortcuts_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network/update/client/release_channel.cpp
  ${VERSION_STRING_CPP}
  game_smoke_test.cpp
)

# SettingsCache is still reached from the game logic, but it lives in the application sources
# rather than in libcockatrice_settings, so the client settings sources are compiled in here
# exactly like loader_local_matching_test and theme_migration_test do.
target_include_directories(game_smoke_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)

target_link_libraries(
  game_smoke_test
  PRIVATE libcockatrice_game
  PRIVATE Threads::Threads
  PRIVATE ${GTEST_BOTH_LIBRARIES}
  PRIVATE ${TEST_QT_MODULES}
)

add_test(NAME game_smoke_test COMMAND game_smoke_test)

if(NOT GTEST_FOUND)
  add_dependencies(game_smoke_test gtest)
endif()
