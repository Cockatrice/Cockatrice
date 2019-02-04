enable_testing()
add_test(NAME dummy_test COMMAND dummy_test)
add_test(NAME expression_test COMMAND expression_test)

# Find GTest

add_executable(dummy_test dummy_test.cpp)
add_executable(expression_test expression_test.cpp)

find_package(GTest)

if(NOT GTEST_FOUND)
    IF(NOT EXISTS "${CMAKE_BINARY_DIR}/gtest-build")
        message(STATUS "Downloading googletest")
        configure_file("${CMAKE_SOURCE_DIR}/cmake/gtest-CMakeLists.txt.in" "${CMAKE_BINARY_DIR}/gtest-download/CMakeLists.txt")
        execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download )
        execute_process(COMMAND ${CMAKE_COMMAND} --build .
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gtest-download )
    ELSE()
        message(STATUS "GoogleTest directory exists")
    ENDIF()

    # Add gtest directly to our build
    add_subdirectory(${CMAKE_BINARY_DIR}/gtest-src
            ${CMAKE_BINARY_DIR}/gtest-build
            EXCLUDE_FROM_ALL )

    # Add the gtest include directory, since gtest
    # doesn't add that dependency to its gtest target
    target_include_directories(gtest INTERFACE
            "${CMAKE_BINARY_DIR}/gtest-src/include" )

    SET(GTEST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/gtest-src/include")
    SET(GTEST_BOTH_LIBRARIES gtest)
    add_dependencies(dummy_test gtest)
    add_dependencies(expression_test gtest)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(TEST_QT_MODULES Qt5::Widgets)


include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(dummy_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(expression_test cockatrice_common ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_subdirectory(carddatabase)
add_subdirectory(loading_from_clipboard)
