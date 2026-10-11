add_definitions("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

cockatrice_add_test(
  NAME
  loading_from_clipboard_test
  SOURCES
  ${VERSION_STRING_CPP}
  clipboard_testing.cpp
  loading_from_clipboard_test.cpp
  LIBS
  libcockatrice_deck_list
  libcockatrice_card
)
