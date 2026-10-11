# Manual benchmark hitting the real card image hosts. Not registered with
# add_test(): it requires a cards.xml, touches the network for minutes at a
# time, and needs network access. Build it explicitly and run by hand.
add_executable(
  picture_loader_benchmark_test
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_local.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_to_load.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_worker.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_worker_work.cpp
  ../../cockatrice/src/client/settings/cache_settings.h
  picture_loader_benchmark.cpp
  settings_cache_mock.cpp
)

target_include_directories(picture_loader_benchmark_test PRIVATE ${CMAKE_SOURCE_DIR}/cockatrice/src)

target_link_libraries(
  picture_loader_benchmark_test
  libcockatrice_card
  libcockatrice_settings
  libcockatrice_interfaces
  libcockatrice_utility
  Threads::Threads
  ${TEST_QT_MODULES}
)
