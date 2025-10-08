set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(abstract)
add_subdirectory(local)
add_subdirectory(remote)

add_library(
    libcockatrice_network_client INTERFACE
)

target_include_directories(libcockatrice_network_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    libcockatrice_network_client PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_network_client_abstract libcockatrice_network_client_local libcockatrice_network_client_remote
)
