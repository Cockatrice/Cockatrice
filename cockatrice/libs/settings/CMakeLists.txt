set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(HEADERS
    include/settings/cache_settings.h
    include/settings/card_counter_settings.h
    include/settings/card_database_settings.h
    include/settings/card_override_settings.h
    include/settings/debug_settings.h
    include/settings/download_settings.h
    include/settings/game_filters_settings.h
    include/settings/layouts_settings.h
    include/settings/message_settings.h
    include/settings/recents_settings.h
    include/settings/servers_settings.h
    include/settings/settings_manager.h
    include/settings/shortcut_treeview.h
    include/settings/shortcuts_settings.h
)

if (Qt6_FOUND)
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
elseif (Qt5_FOUND)
    qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
endif ()

add_library(settingslib STATIC
    src/cache_settings.cpp
    src/card_counter_settings.cpp
    src/card_database_settings.cpp
    src/card_override_settings.cpp
    src/debug_settings.cpp
    src/download_settings.cpp
    src/game_filters_settings.cpp
    src/layouts_settings.cpp
    src/message_settings.cpp
    src/recents_settings.cpp
    src/servers_settings.cpp
    src/settings_manager.cpp
    src/shortcut_treeview.cpp
    src/shortcuts_settings.cpp
    ${MOC_SOURCES}
)

target_include_directories(settingslib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_SOURCE_DIR}/common
    PUBLIC ${CMAKE_SOURCE_DIR}/cockatrice/libs/utility/include
    PUBLIC ${CMAKE_SOURCE_DIR}/cockatrice/src/client/network
)

target_link_libraries(settingslib
    PUBLIC cockatrice_common ${COCKATRICE_QT_MODULES}
)
