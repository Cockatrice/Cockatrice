# NOTE: Qt modules for tests are defined centrally in cmake/FindQtRuntime.cmake (the _TEST_NEEDED variable).
# If a new test needs additional Qt modules, add them there — not in individual test CMakeLists.txt files.
enable_testing()

include(GoogleTest)

# Find GTest

find_package(GTest)

if(NOT GTEST_FOUND)
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/gtest-build")
    message(STATUS "Downloading googletest")
    configure_file(
      "${CMAKE_SOURCE_DIR}/cmake/gtest-CMakeLists.txt.in" "${CMAKE_BINARY_DIR}/gtest-download/CMakeLists.txt"
    )
    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download
    )
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download)
  else()
    message(STATUS "GoogleTest directory exists")
  endif()

  # Add gtest directly to our build
  add_subdirectory(${CMAKE_BINARY_DIR}/gtest-src ${CMAKE_BINARY_DIR}/gtest-build EXCLUDE_FROM_ALL)

  # Add the gtest include directory, since gtest
  # doesn't add that dependency to its gtest target
  target_include_directories(gtest INTERFACE "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gtest-src/include>")

  set(GTEST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/gtest-src/include")
  # gtest_main is not optional here. FindGTest sets GTEST_BOTH_LIBRARIES to both halves, so this
  # has to match it or the two paths disagree about what provides main(). A test source that
  # defines its own main() gets away with the omission; one that does not fails at link with
  # `undefined reference to main` pointing at crt1.o, which names neither gtest nor this line.
  set(GTEST_BOTH_LIBRARIES gtest gtest_main)
endif()

# Directory-scoped on purpose: the test subdirectories below add targets that include the
# fetched gtest headers and do not repeat this path themselves.
include_directories(${GTEST_INCLUDE_DIRS})

# --- Test registration ----------------------------------------------------------
#
# Every test target needs the same four steps: build the executable, depend on gtest when it
# was fetched from source, link the shared test libraries, and hand the cases to CTest. The
# `if(NOT GTEST_FOUND)` branch is the easy one to forget — omit it and the link fails with
# undefined gtest symbols — so it lives here once instead of being repeated per test.
#
# This also replaces `add_test(NAME x COMMAND x)`. Naming the binary as a single CTest entry
# hides which case failed and makes `ctest -R Suite.Case` impossible; discovery exposes each
# TEST() as its own CTest entry instead.
#
# Labels: exactly one per test, because gtest_discover_tests() cannot carry more than one.
# GoogleTestAddTests.cmake accumulates the generated script with `foreach(arg ${ARGN})`, so a
# label list serialises as bare `LABELS ci server` in the CTestTestfile, and CTest reads only
# the first value of a property and silently discards the rest. A label therefore classifies a
# test into exactly one group, and the three groups cover every selection worth making:
#
#   ci      the default hermetic set, so `ctest -LE bench` runs everything but benchmarks
#   server  servatrice-facing tests, so `ctest -L server` works
#   bench   the slow suites, kept out of the default run
#
# A second label is a configure-time FATAL_ERROR rather than a line CTest quietly reads as one.
# Anything orthogonal to a test's kind belongs in a helper flag beside the label, so it
# composes with the group instead of competing for the single slot. NO_SANITIZE is such a flag.
#
# TIMEOUT is bounded per test because CTest's implicit default is 1500s: one hung socket wait
# would otherwise stall CI for 25 minutes.
#
# EXTRA_LIBS is appended after the shared libraries so a caller adding a static archive (zlib,
# lzma) keeps it last, which is where the linker needs it.

function(cockatrice_add_test)
  cmake_parse_arguments(
    PARSE_ARGV 0 ARG "MANUAL;NO_SANITIZE" "NAME" "SOURCES;LIBS;LABELS;DEFINITIONS;INCLUDE_DIRS;EXTRA_LIBS"
  )

  if(NOT ARG_NAME OR NOT ARG_SOURCES)
    message(FATAL_ERROR "cockatrice_add_test requires at least NAME and SOURCES")
  endif()

  add_executable(${ARG_NAME} ${ARG_SOURCES})

  if(ARG_INCLUDE_DIRS)
    target_include_directories(${ARG_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
  endif()

  if(ARG_DEFINITIONS)
    target_compile_definitions(${ARG_NAME} PRIVATE ${ARG_DEFINITIONS})
  endif()

  target_link_libraries(
    ${ARG_NAME} ${ARG_LIBS} Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES} ${ARG_EXTRA_LIBS}
  )

  # Only needed when gtest was downloaded and add_subdirectory'd above. An imported gtest
  # target already carries its own dependency, so this must stay conditional.
  if(NOT GTEST_FOUND)
    add_dependencies(${ARG_NAME} gtest gtest_main)
  endif()

  # MANUAL targets are built but deliberately kept out of ctest: benchmarks that need a card
  # database, hit the network, or measure RAM. Leaving them unregistered keeps CI hermetic
  # and fast. They still link gtest so they can use TEST() without a second code path.
  if(ARG_MANUAL)
    return()
  endif()

  # NO_SANITIZE marks a target the sanitizer leg cannot run. Excluded by not handing its cases
  # to CTest at all, rather than by a label, so the exclusion cannot rot into a line CTest
  # quietly reads as something else. The target still builds, and still runs on the other legs.
  if(ARG_NO_SANITIZE AND COCKATRICE_SANITIZERS)
    return()
  endif()

  # One label only, per the note above. Rejecting a second loudly beats emitting a
  # CTestTestfile line that CTest will quietly read as one label.
  list(LENGTH ARG_LABELS _label_count)
  if(_label_count GREATER 1)
    message(FATAL_ERROR "${ARG_NAME}: pass at most one label, got '${ARG_LABELS}'")
  endif()
  if(NOT ARG_LABELS)
    set(ARG_LABELS ci)
  endif()

  # GoogleTestAddTests defaults DISCOVERY_TIMEOUT to 5s, which is ample for a plain debug
  # binary (--gtest_list_tests costs 0.04s) but not for an instrumented one (~2.8s cold, and
  # discovery runs several binaries concurrently behind a -j build). Left at the default, a
  # sanitizer build fails at the *link* step with a bare "Process terminated due to timeout"
  # and no hint that discovery, not the test, is what is slow.
  if(COCKATRICE_SANITIZERS)
    set(_discovery_timeout 300)
  else()
    set(_discovery_timeout 60)
  endif()

  gtest_discover_tests(
    ${ARG_NAME} DISCOVERY_MODE POST_BUILD DISCOVERY_TIMEOUT ${_discovery_timeout}
    PROPERTIES LABELS "${ARG_LABELS}"
    TIMEOUT 60
  )
endfunction()

# Same wiring for targets that are built but deliberately kept out of ctest: benchmarks that
# need a card database, hit the network, or measure RAM. A separate name rather than a MANUAL
# flag spelled out at each call site, so "this is not in the CI suite" is visible in the CMake
# file. Note the unquoted expansions: an unset multi-value keyword expands to zero arguments,
# which is what lets this forward a caller's keywords verbatim without tripping the FATAL_ERROR
# guard above on an empty list.
function(cockatrice_add_manual_test)
  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "NAME" "SOURCES;LIBS;DEFINITIONS;INCLUDE_DIRS;EXTRA_LIBS")

  cockatrice_add_test(
    NAME
    ${ARG_NAME}
    SOURCES
    ${ARG_SOURCES}
    LIBS
    ${ARG_LIBS}
    DEFINITIONS
    ${ARG_DEFINITIONS}
    INCLUDE_DIRS
    ${ARG_INCLUDE_DIRS}
    EXTRA_LIBS
    ${ARG_EXTRA_LIBS}
    MANUAL
  )
endfunction()

# --- Unit tests ------------------------------------------------------------------

cockatrice_add_test(NAME expression_test SOURCES expression_test.cpp LIBS libcockatrice_utility)
cockatrice_add_test(NAME clamped_arithmetic_test SOURCES clamped_arithmetic_test.cpp LIBS libcockatrice_utility)
cockatrice_add_test(NAME test_age_formatting SOURCES test_age_formatting.cpp LIBS libcockatrice_utility)
cockatrice_add_test(NAME password_hash_test SOURCES password_hash_test.cpp LIBS libcockatrice_utility)
cockatrice_add_test(NAME report_categories_test SOURCES report_categories_test.cpp LIBS libcockatrice_utility)
cockatrice_add_test(NAME warning_categories_test SOURCES warning_categories_test.cpp LIBS libcockatrice_utility)
cockatrice_add_test(
  NAME
  playmat_resolver_test
  SOURCES
  playmat_resolver_test.cpp
  LIBS
  libcockatrice_deck_list
  libcockatrice_utility
)
cockatrice_add_test(
  NAME
  server_rate_limiter_test
  SOURCES
  server_rate_limiter_test.cpp
  LIBS
  libcockatrice_utility
  LABELS
  server
)
cockatrice_add_test(
  NAME
  server_card_counter_test
  SOURCES
  server_card_counter_test.cpp
  LIBS
  libcockatrice_network
  LABELS
  server
)
cockatrice_add_test(
  NAME
  server_counter_test
  SOURCES
  server_counter_test.cpp
  LIBS
  libcockatrice_network
  LABELS
  server
)
cockatrice_add_test(
  NAME
  server_developer_role_test
  NO_SANITIZE
  SOURCES
  server_developer_role_test.cpp
  LIBS
  libcockatrice_network
  libcockatrice_rng
  LABELS
  server
)
cockatrice_add_test(
  NAME
  server_game_join_test
  SOURCES
  server_game_join_test.cpp
  LIBS
  libcockatrice_network_server_remote
  libcockatrice_rng
  LABELS
  server
)
cockatrice_add_test(
  NAME
  latency_tracker_test
  SOURCES
  latency_tracker_test.cpp
  LIBS
  libcockatrice_network
  LABELS
  server
)
cockatrice_add_test(
  NAME
  lag_monitor_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/lag_monitor.cpp
  lag_monitor_test.cpp
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src
)
cockatrice_add_test(
  NAME
  metrics_registry_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/servatrice/src/metrics_registry.cpp
  metrics_registry_test.cpp
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/servatrice/src
  LABELS
  server
)
cockatrice_add_test(
  NAME
  card_update_progress_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/card_database_update/card_update_progress.cpp
  card_update_progress_test.cpp
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src
)
cockatrice_add_test(
  NAME
  loader_local_matching_test
  SOURCES
  ${CMAKE_SOURCE_DIR}/cockatrice/src/interface/card_picture_loader/card_picture_loader_local.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/cache_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/card_counter_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/settings/shortcuts_settings.cpp
  ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network/update/client/release_channel.cpp
  ${VERSION_STRING_CPP}
  loader_local_matching_test.cpp
  LIBS
  libcockatrice_settings
  INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/cockatrice/src
)
cockatrice_add_test(
  NAME
  deck_hash_performance_test
  SOURCES
  deck_hash_performance_test.cpp
  LIBS
  libcockatrice_deck_list
  libcockatrice_utility
  LABELS
  bench
)

# The translation loader test compiles a .qm fixture, which needs lrelease from Qt Linguist.
if(TARGET Qt6::lrelease)
  set(TRANSLATION_LOADER_TEST_QM "${CMAKE_CURRENT_BINARY_DIR}/translation_loader_test_de.qm")
  add_custom_command(
    OUTPUT "${TRANSLATION_LOADER_TEST_QM}"
    COMMAND Qt6::lrelease "${CMAKE_CURRENT_SOURCE_DIR}/translation_loader_test.ts" -qm "${TRANSLATION_LOADER_TEST_QM}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/translation_loader_test.ts"
    COMMENT "Compiling translation_loader_test.ts"
  )
  add_custom_target(translation_loader_test_qm DEPENDS "${TRANSLATION_LOADER_TEST_QM}")

  cockatrice_add_test(
    NAME
    translation_loader_test
    SOURCES
    translation_loader_test.cpp
    LIBS
    libcockatrice_utility
    DEFINITIONS
    TRANSLATION_LOADER_TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  )
  add_dependencies(translation_loader_test translation_loader_test_qm)

  # A multi-config generator (MSBuild, Xcode) puts the executable in a per-config subdirectory while
  # the .qm stays in the current binary directory, so copy the fixture next to the binary: that is the
  # layout ABuildTreeFindsTheTranslationsNextToTheExecutable is about, and the only one it finds on
  # those generators.
  add_custom_command(
    TARGET translation_loader_test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TRANSLATION_LOADER_TEST_QM}"
            "$<TARGET_FILE_DIR:translation_loader_test>/"
    VERBATIM
  )
endif()

# Guard must match the condition for src/single_instance_manager.cpp in cockatrice/CMakeLists.txt.
if(WITH_CLIENT)
  cockatrice_add_test(
    NAME
    single_instance_manager_test
    SOURCES
    ${CMAKE_SOURCE_DIR}/cockatrice/src/single_instance_manager.cpp
    single_instance_manager_test.cpp
    INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/cockatrice/src
    ${CMAKE_SOURCE_DIR}/libcockatrice_utility
  )
endif()

add_subdirectory(card_zone_algorithms)
add_subdirectory(carddatabase)
add_subdirectory(deck_list_model)
add_subdirectory(deck_list_zones)
add_subdirectory(loading_from_clipboard)
add_subdirectory(movecard_tests)
add_subdirectory(oracle)
add_subdirectory(settings)

# theme_migration_test compiles client sources, whose headers are only reachable from a client build.
if(WITH_CLIENT)
  add_subdirectory(theme_migration)
endif()

# picture_loader_benchmark links libcockatrice_settings, which only exists when
# a client/UI-capable target is being built.
if(WITH_ORACLE OR WITH_CLIENT)
  add_subdirectory(picture_loader_benchmark)
endif()
