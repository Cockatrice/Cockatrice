set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(libcockatrice/client)

add_library(
    libcockatrice_network INTERFACE
)


target_include_directories(libcockatrice_network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    libcockatrice_network PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_network_client
)
