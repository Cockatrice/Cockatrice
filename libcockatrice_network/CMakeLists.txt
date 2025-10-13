set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(libcockatrice/network/client)
add_subdirectory(libcockatrice/network/server)

add_library(libcockatrice_network INTERFACE)

target_include_directories(libcockatrice_network INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_network INTERFACE libcockatrice_network_client libcockatrice_network_server)
include_directories(${${COCKATRICE_QT_VERSION_NAME}Core_INCLUDE_DIRS})
