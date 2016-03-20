ADD_DEFINITIONS("-DCARDDB_DATADIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")
add_executable(carddatabase_test
    carddatabase_test.cpp
    ../../cockatrice/src/carddatabase.cpp

)
if(NOT GTEST_FOUND)
    add_dependencies(carddatabase_test gtest)
endif()
target_link_libraries(carddatabase_test ${GTEST_BOTH_LIBRARIES})
add_test(NAME carddatabase_test COMMAND carddatabase_test)

# Qt4 stuff
if(Qt4_FOUND)
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTMULTIMEDIA TRUE)

    # Include directories
    INCLUDE(${QT_USE_FILE})
    INCLUDE_DIRECTORIES(${QT_INCLUDES})
    TARGET_LINK_LIBRARIES(carddatabase_test {QT_LIBRARIES})
endif()

# qt5 stuff
if(Qt5Widgets_FOUND)
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

    qt5_use_modules(carddatabase_test ${COCKATRICE_LIBS})
endif()