ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(loading_from_clipboard_test
        loading_from_clipboard_test.cpp
        ../../common/decklist.cpp
        )

if(NOT GTEST_FOUND)
    add_dependencies(loading_from_clipboard_test gtest)
endif()

target_link_libraries(loading_from_clipboard_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(loading_from_clipboard_test cockatrice_common)

add_test(NAME loading_from_clipboard_test COMMAND loading_from_clipboard_test)

#### I feel like the rest of this file should not be necessary and
#### is (effective) cargo culting of tests/carddatabase/CMakeLists.txt.
#### Ideally we would only need to say "hey this test is against something from cockatrice_common",
#### and cockatrice_common would declare all of it's dependencies. I need to learn more about CMake.

# qt5 stuff
include_directories(${Qt5Widgets_INCLUDE_DIRS})
list(APPEND COCKATRICE_LIBS Widgets)

# QtConcurrent
find_package(Qt5Concurrent)
if(Qt5Concurrent_FOUND)
    include_directories(${Qt5Concurrent_INCLUDE_DIRS})
    list(APPEND ORACLE_LIBS Concurrent)
endif()

# QtNetwork
find_package(Qt5Network)
if(Qt5Network_FOUND)
    include_directories(${Qt5Network_INCLUDE_DIRS})
    list(APPEND COCKATRICE_LIBS Network)
endif()

qt5_use_modules(loading_from_clipboard_test ${COCKATRICE_LIBS})