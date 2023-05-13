enable_testing()
add_test(NAME dummy_test COMMAND dummy_test)
add_test(NAME expression_test COMMAND expression_test)

add_test(NAME test_age_formatting COMMAND test_age_formatting)
add_test(NAME password_hash_test COMMAND password_hash_test)

# Find GTest

add_executable(dummy_test dummy_test.cpp)
add_executable(expression_test expression_test.cpp)
add_executable(test_age_formatting test_age_formatting.cpp)
add_executable(password_hash_test password_hash_test.cpp)

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
  set(GTEST_BOTH_LIBRARIES gtest)
  add_dependencies(dummy_test gtest)
  add_dependencies(expression_test gtest)
  add_dependencies(test_age_formatting gtest)
  add_dependencies(password_hash_test gtest)
endif()

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(dummy_test Threads::Threads ${GTEST_BOTH_LIBRARIES})
target_link_libraries(expression_test cockatrice_common Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(test_age_formatting Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})
target_link_libraries(password_hash_test cockatrice_common Threads::Threads ${GTEST_BOTH_LIBRARIES} ${TEST_QT_MODULES})

add_subdirectory(carddatabase)
add_subdirectory(loading_from_clipboard)
