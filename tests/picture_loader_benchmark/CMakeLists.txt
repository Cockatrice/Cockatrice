# Manual benchmark hitting the real card image hosts. Not registered with CTest: it requires a
# cards.xml, touches the network for minutes at a time, and needs network access. Build it
# explicitly and run by hand.
cockatrice_add_manual_test(
  NAME
  picture_loader_benchmark_test
  SOURCES
  ${VERSION_STRING_CPP}
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_local.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_to_load.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_worker.cpp
  ../../cockatrice/src/interface/card_picture_loader/card_picture_loader_worker_work.cpp
  ../../cockatrice/src/client/settings/cache_settings.h
  picture_loader_benchmark.cpp
  settings_cache_mock.cpp
  LIBS
  libcockatrice_card
  libcockatrice_settings
  libcockatrice_interfaces
  libcockatrice_utility
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src
  MANUAL
)
