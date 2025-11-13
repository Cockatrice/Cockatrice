set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    libcockatrice/interfaces/interface_card_database_path_provider.h
    libcockatrice/interfaces/interface_card_preference_provider.h
    libcockatrice/interfaces/interface_card_set_priority_controller.h
    libcockatrice/interfaces/interface_network_settings_provider.h
    libcockatrice/interfaces/noop_card_database_path_provider.h
    libcockatrice/interfaces/noop_card_preference_provider.h
    libcockatrice/interfaces/noop_card_set_priority_controller.h
)

if(Qt6_FOUND)
  qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif(Qt5_FOUND)
  qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif()

add_library(libcockatrice_interfaces STATIC ${MOC_SOURCES})

target_include_directories(libcockatrice_interfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_interfaces PUBLIC ${QT_CORE_MODULE})
