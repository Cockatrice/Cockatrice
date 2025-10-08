set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS remote_client.h)

set(SOURCES remote_client.cpp)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(libcockatrice_network_client_remote STATIC ${MOC_SOURCES} ${SOURCES})

add_dependencies(libcockatrice_network_client_remote libcockatrice_network_client_abstract libcockatrice_protocol)

target_include_directories(libcockatrice_network_client_remote PUBLIC .)

target_link_libraries(
  libcockatrice_network_client_remote PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_network_client_abstract
                                             libcockatrice_settings libcockatrice_utility libcockatrice_protocol
)
