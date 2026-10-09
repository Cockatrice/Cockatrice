set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS abstract_client.h latency_tracker.h)

set(SOURCES abstract_client.cpp latency_tracker.cpp)

qt6_wrap_cpp(MOC_SOURCES ${HEADERS})

add_library(libcockatrice_network_client_abstract STATIC ${MOC_SOURCES} ${SOURCES})

add_dependencies(libcockatrice_network_client_abstract libcockatrice_protocol libcockatrice_network_server_remote)

target_include_directories(libcockatrice_network_client_abstract PUBLIC .)

target_link_libraries(
  libcockatrice_network_client_abstract PUBLIC ${QT_MAIN_VERSION_STRING}::Network ${QT_MAIN_VERSION_STRING}::WebSockets
                                               libcockatrice_protocol libcockatrice_network_server_remote
)
