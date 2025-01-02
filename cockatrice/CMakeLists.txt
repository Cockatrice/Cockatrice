# CMakeLists for cockatrice directory
#
# provides the cockatrice binary

project(Cockatrice VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(cockatrice_SOURCES
    src/game/cards/abstract_card_drag_item.cpp
    src/game/cards/abstract_card_item.cpp
    src/client/game_logic/abstract_client.cpp
    src/game/board/abstract_counter.cpp
    src/game/board/abstract_graphics_item.cpp
    src/game/board/arrow_item.cpp
    src/game/board/arrow_target.cpp
    src/game/cards/card_database.cpp
    src/game/cards/card_database_manager.cpp
    src/game/cards/card_database_model.cpp
    src/game/cards/card_database_parser/card_database_parser.cpp
    src/game/cards/card_database_parser/cockatrice_xml_3.cpp
    src/game/cards/card_database_parser/cockatrice_xml_4.cpp
    src/game/cards/card_drag_item.cpp
    src/game/filters/filter_card.cpp
    src/client/ui/widgets/cards/card_info_frame_widget.cpp
    src/client/ui/widgets/cards/card_info_picture_widget.cpp
    src/client/ui/widgets/cards/card_info_text_widget.cpp
    src/client/ui/widgets/cards/card_info_display_widget.cpp
    src/client/ui/widgets/cards/card_size_widget.cpp
    src/game/cards/card_item.cpp
    src/game/cards/card_list.cpp
    src/game/zones/card_zone.cpp
    src/server/chat_view/chat_view.cpp
    src/game/board/counter_general.cpp
    src/deck/custom_line_edit.cpp
    src/deck/deck_loader.cpp
    src/deck/deck_list_model.cpp
    src/deck/deck_stats_interface.cpp
    src/deck/deck_view.cpp
    src/dialogs/dlg_connect.cpp
    src/dialogs/dlg_create_token.cpp
    src/dialogs/dlg_create_game.cpp
    src/dialogs/dlg_edit_avatar.cpp
    src/dialogs/dlg_edit_password.cpp
    src/dialogs/dlg_edit_tokens.cpp
    src/dialogs/dlg_edit_user.cpp
    src/dialogs/dlg_filter_games.cpp
    src/dialogs/dlg_forgot_password_challenge.cpp
    src/dialogs/dlg_forgot_password_request.cpp
    src/dialogs/dlg_forgot_password_reset.cpp
    src/dialogs/dlg_load_deck_from_clipboard.cpp
    src/dialogs/dlg_load_remote_deck.cpp
    src/dialogs/dlg_manage_sets.cpp
    src/dialogs/dlg_move_top_cards_until.cpp
    src/dialogs/dlg_register.cpp
    src/dialogs/dlg_roll_dice.cpp
    src/dialogs/dlg_settings.cpp
    src/dialogs/dlg_tip_of_the_day.cpp
    src/dialogs/dlg_update.cpp
    src/dialogs/dlg_view_log.cpp
    src/game/filters/filter_string.cpp
    src/game/filters/filter_builder.cpp
    src/game/filters/filter_tree.cpp
    src/game/filters/filter_tree_model.cpp
    src/client/ui/layouts/flow_layout.cpp
    src/client/ui/layouts/horizontal_flow_layout.cpp
    src/client/ui/layouts/vertical_flow_layout.cpp
    src/client/ui/widgets/general/layout_containers/flow_widget.cpp
    src/game/game_scene.cpp
    src/game/game_selector.cpp
    src/game/games_model.cpp
    src/game/game_view.cpp
    src/client/get_text_with_max.cpp
    src/game/hand_counter.cpp
    src/server/handle_public_servers.cpp
    src/game/zones/hand_zone.cpp
    src/client/game_logic/key_signals.cpp
    src/client/ui/line_edit_completer.cpp
    src/server/local_client.cpp
    src/server/local_server.cpp
    src/server/local_server_interface.cpp
    src/utility/logger.cpp
    src/client/ui/widgets/cards/card_info_picture_enlarged_widget.cpp
    src/client/ui/widgets/cards/card_info_picture_with_text_overlay_widget.cpp
    src/client/ui/widgets/general/display/labeled_input.cpp
    src/client/ui/widgets/general/display/dynamic_font_size_label.cpp
    src/client/ui/widgets/general/display/dynamic_font_size_push_button.cpp
    src/client/ui/widgets/general/display/shadow_background_label.cpp
    src/main.cpp
    src/server/message_log_widget.cpp
    src/client/ui/layouts/overlap_layout.cpp
    src/client/ui/widgets/general/layout_containers/overlap_widget.cpp
    src/client/ui/widgets/general/layout_containers/overlap_control_widget.cpp
    src/server/pending_command.cpp
    src/game/phase.cpp
    src/client/ui/phases_toolbar.cpp
    src/client/ui/picture_loader.cpp
    src/game/zones/pile_zone.cpp
    src/client/ui/pixel_map_generator.cpp
    src/game/player/player.cpp
    src/game/player/player_list_widget.cpp
    src/game/player/player_target.cpp
    src/client/ui/widgets/printing_selector/all_zones_card_amount_widget.cpp
    src/client/ui/widgets/printing_selector/card_amount_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector.cpp
    src/client/ui/widgets/printing_selector/printing_selector_card_display_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_card_overlay_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_card_search_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_card_selection_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_card_sorting_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_view_options_toolbar_widget.cpp
    src/client/ui/widgets/printing_selector/printing_selector_view_options_widget.cpp
    src/client/ui/widgets/printing_selector/set_name_and_collectors_number_display_widget.cpp
    src/client/network/release_channel.cpp
    src/client/network/client_update_checker.cpp
    src/server/remote/remote_client.cpp
    src/server/remote/remote_decklist_tree_widget.cpp
    src/server/remote/remote_replay_list_tree_widget.cpp
    src/client/network/replay_timeline_widget.cpp
    src/game/zones/select_zone.cpp
    src/utility/sequence_edit.cpp
    src/client/network/sets_model.cpp
    src/settings/card_database_settings.cpp
    src/settings/download_settings.cpp
    src/settings/game_filters_settings.cpp
    src/settings/layouts_settings.cpp
    src/settings/message_settings.cpp
    src/settings/recents_settings.cpp
    src/settings/servers_settings.cpp
    src/settings/settings_manager.cpp
    src/settings/cache_settings.cpp
    src/settings/shortcuts_settings.cpp
    src/settings/shortcut_treeview.cpp
    src/settings/card_override_settings.cpp
    src/settings/debug_settings.cpp
    src/client/sound_engine.cpp
    src/client/network/spoiler_background_updater.cpp
    src/game/zones/stack_zone.cpp
    src/client/tabs/tab.cpp
    src/client/tabs/tab_account.cpp
    src/client/tabs/tab_admin.cpp
    src/client/tabs/tab_deck_editor.cpp
    src/client/tabs/tab_deck_storage.cpp
    src/client/tabs/tab_game.cpp
    src/client/tabs/tab_logs.cpp
    src/client/tabs/tab_message.cpp
    src/client/tabs/tab_replays.cpp
    src/client/tabs/tab_room.cpp
    src/client/tabs/tab_server.cpp
    src/client/tabs/tab_supervisor.cpp
    src/game/zones/table_zone.cpp
    src/client/tapped_out_interface.cpp
    src/client/ui/theme_manager.cpp
    src/client/ui/tip_of_the_day.cpp
    src/client/translate_counter_name.cpp
    src/client/update_downloader.cpp
    src/server/user/user_context_menu.cpp
    src/server/user/user_info_connection.cpp
    src/server/user/user_info_box.cpp
    src/server/user/user_list.cpp
    src/client/ui/window_main.cpp
    src/game/zones/view_zone_widget.cpp
    src/game/zones/view_zone.cpp
    ${VERSION_STRING_CPP}
)

add_subdirectory(sounds)
add_subdirectory(themes)

set(cockatrice_RESOURCES cockatrice.qrc)

if(UPDATE_TRANSLATIONS)
  file(GLOB_RECURSE translate_cockatrice_SRCS ${CMAKE_SOURCE_DIR}/cockatrice/src/*.cpp
       ${CMAKE_SOURCE_DIR}/cockatrice/src/*.h
  )
  file(GLOB_RECURSE translate_common_SRCS ${CMAKE_SOURCE_DIR}/common/*.cpp ${CMAKE_SOURCE_DIR}/common/*.h)
  set(translate_SRCS ${translate_cockatrice_SRCS} ${translate_common_SRCS})
  set(cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice_en@source.ts")
else()
  file(GLOB cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
endif(UPDATE_TRANSLATIONS)

if(WIN32)
  set(cockatrice_SOURCES ${cockatrice_SOURCES} cockatrice.rc)
endif(WIN32)

if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set(cockatrice_SOURCES ${cockatrice_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.icns)
endif(APPLE)

if(Qt6_FOUND)
  qt6_add_resources(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})
elseif(Qt5_FOUND)
  qt5_add_resources(cockatrice_RESOURCES_RCC ${cockatrice_RESOURCES})
endif()

# Declare path variables
set(ICONDIR
    share/icons
    CACHE STRING "icon dir"
)
set(DESKTOPDIR
    share/applications
    CACHE STRING "desktop file destination"
)

# Include directories
include_directories(../common)
include_directories(${PROTOBUF_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/common)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(COCKATRICE_MAC_QM_INSTALL_DIR "cockatrice.app/Contents/Resources/translations")
set(COCKATRICE_UNIX_QM_INSTALL_DIR "share/cockatrice/translations")
set(COCKATRICE_WIN32_QM_INSTALL_DIR "translations")

if(Qt6_FOUND)
  qt6_add_executable(
    cockatrice
    WIN32
    MACOSX_BUNDLE
    ${cockatrice_SOURCES}
    ${cockatrice_RESOURCES_RCC}
    ${cockatrice_MOC_SRCS}
    MANUAL_FINALIZATION
  )
elseif(Qt5_FOUND)
  # Qt5 Translations need to be linked at executable creation time
  if(Qt5LinguistTools_FOUND)
    if(UPDATE_TRANSLATIONS)
      qt5_create_translation(cockatrice_QM ${translate_SRCS} ${cockatrice_TS})
    else()
      qt5_add_translation(cockatrice_QM ${cockatrice_TS})
    endif()
  endif()
  add_executable(
    cockatrice WIN32 MACOSX_BUNDLE ${cockatrice_MOC_SRCS} ${cockatrice_QM} ${cockatrice_RESOURCES_RCC}
                                   ${cockatrice_SOURCES}
  )
  if(UNIX)
    if(APPLE)
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

if(Qt5_FOUND)
  target_link_libraries(cockatrice cockatrice_common ${COCKATRICE_QT_MODULES})
else()
  target_link_libraries(cockatrice PUBLIC cockatrice_common ${COCKATRICE_QT_MODULES})
endif()

if(UNIX)
  if(APPLE)
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.cockatrice.${PROJECT_NAME}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

    set_target_properties(cockatrice PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

    install(TARGETS cockatrice BUNDLE DESTINATION ./)
  else()
    # Assume linux
    install(TARGETS cockatrice RUNTIME DESTINATION bin/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.png DESTINATION ${ICONDIR}/hicolor/48x48/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/cockatrice.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cockatrice.desktop DESTINATION ${DESKTOPDIR})
  endif()
elseif(WIN32)
  install(TARGETS cockatrice RUNTIME DESTINATION ./)
endif()

if(APPLE)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir cockatrice.app/Contents/Plugins)
  set(qtconf_dest_dir cockatrice.app/Contents/Resources)

  # Qt plugins: audio (Qt5), iconengines, imageformats, platforms, printsupport (Qt5), styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "*.dSYM" EXCLUDE
    PATTERN "*_debug.dylib" EXCLUDE
    PATTERN "audio/*.dylib"
    PATTERN "iconengines/*.dylib"
    PATTERN "imageformats/*.dylib"
    PATTERN "platforms/*.dylib"
    PATTERN "printsupport/*.dylib"
    PATTERN "styles/*.dylib"
    PATTERN "tls/*.dylib"
  )

  install(
    CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/cockatrice.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        "
    COMPONENT Runtime
  )
endif()

if(WIN32)
  # these needs to be relative to CMAKE_INSTALL_PREFIX
  set(plugin_dest_dir Plugins)
  set(qtconf_dest_dir .)

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE}/"
    DESTINATION ./
    FILES_MATCHING
    PATTERN "*.dll"
  )

  # Qt plugins: audio (Qt5), iconengines, imageformats, platforms, printsupport (Qt5), styles, tls (Qt6)
  install(
    DIRECTORY "${QT_PLUGINS_DIR}/"
    DESTINATION ${plugin_dest_dir}
    COMPONENT Runtime
    FILES_MATCHING
    PATTERN "audio/qtaudio_wasapi.dll"
    PATTERN "audio/qtaudio_windows.dll"
    PATTERN "iconengines/qsvgicon.dll"
    PATTERN "imageformats/*.dll"
    PATTERN "mediaservice/dsengine.dll"
    PATTERN "mediaservice/wmfengine.dll"
    PATTERN "platforms/qdirect2d.dll"
    PATTERN "platforms/qminimal.dll"
    PATTERN "platforms/qoffscreen.dll"
    PATTERN "platforms/qwindows.dll"
    PATTERN "printsupport/windowsprintersupport.dll"
    PATTERN "styles/qcertonlybackend.dll"
    PATTERN "styles/qopensslbackend.dll"
    PATTERN "styles/qschannelbackend.dll"
    PATTERN "styles/qwindowsvistastyle.dll"
    PATTERN "tls/qcertonlybackend.dll"
    PATTERN "tls/qopensslbackend.dll"
    PATTERN "tls/qschannelbackend.dll"
  )

  install(
    CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    "
    COMPONENT Runtime
  )

  install(
    CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/Cockatrice.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        "
    COMPONENT Runtime
  )

  if(OPENSSL_FOUND)
    install(FILES ${OPENSSL_INCLUDE_DIRS} DESTINATION ./)
  endif()
endif()

if(Qt6_FOUND AND Qt6LinguistTools_FOUND)
  #Qt6 Translations happen after the executable is built up
  if(UPDATE_TRANSLATIONS)
    qt6_add_translations(
      cockatrice
      TS_FILES
      ${cockatrice_TS}
      SOURCES
      ${translate_SRCS}
      QM_FILES_OUTPUT_VARIABLE
      cockatrice_QM
    )
  else()
    qt6_add_translations(cockatrice TS_FILES ${cockatrice_TS} QM_FILES_OUTPUT_VARIABLE cockatrice_QM)
  endif()

  if(UNIX)
    if(APPLE)
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_MAC_QM_INSTALL_DIR})
    else()
      install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_UNIX_QM_INSTALL_DIR})
    endif()
  elseif(WIN32)
    install(FILES ${cockatrice_QM} DESTINATION ${COCKATRICE_WIN32_QM_INSTALL_DIR})
  endif()
endif()

if(Qt6_FOUND)
  qt6_finalize_target(cockatrice)
endif()
