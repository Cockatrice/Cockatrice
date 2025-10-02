cmake_minimum_required(VERSION 3.16)
project(Utility VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UTILITY_SOURCES src/key_signals.cpp src/levenshtein.cpp src/logger.cpp)

set(UTILITY_HEADERS include/utility/key_signals.h include/utility/levenshtein.h include/utility/logger.h
                    include/utility/macros.h
)

qt_add_library(utility STATIC ${UTILITY_SOURCES} ${UTILITY_HEADERS})

target_include_directories(utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(utility PUBLIC ${COCKATRICE_QT_MODULES})
