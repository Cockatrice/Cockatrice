add_executable(
  theme_migration_test
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/theme_manager.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/theme_config.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/pixel_map_generator.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/cache_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/card_counter_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/shortcuts_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network/update/client/release_channel.cpp
  ${VERSION_STRING_CPP}
  theme_migration_test.cpp
)
target_include_directories(theme_migration_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)

target_link_libraries(
  theme_migration_test
  libcockatrice_settings
  libcockatrice_network
  libcockatrice_network_server_remote
  libcockatrice_protocol
  Threads::Threads
  ${GTEST_BOTH_LIBRARIES}
  ${QT_MODULES_TEST}
)

add_test(NAME theme_migration_test COMMAND theme_migration_test)

if(NOT GTEST_FOUND)
  add_dependencies(theme_migration_test gtest)
endif()
