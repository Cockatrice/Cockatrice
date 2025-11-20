# CMakeLists for cockatrice directory
#
# provides the cockatrice binary

project(Cockatrice VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(cockatrice_SOURCES
    ${VERSION_STRING_CPP}
    # sort by alphabetical order, so that there is no debate about where to add new sources to the list
    src/client/network/update/client/update_downloader.cpp
    src/client/network/interfaces/deck_stats_interface.cpp
    src/client/network/interfaces/tapped_out_interface.cpp
    src/client/network/parsers/deck_link_to_api_transformer.cpp
    src/client/network/update/client/client_update_checker.cpp
    src/client/network/update/client/release_channel.cpp
    src/client/network/update/card_spoiler/spoiler_background_updater.cpp
    src/client/sound_engine.cpp
    src/client/settings/cache_settings.cpp
    src/client/settings/card_counter_settings.cpp
    src/client/settings/shortcut_treeview.cpp
    src/client/settings/shortcuts_settings.cpp
    src/interface/deck_loader/deck_loader.cpp
    src/interface/widgets/dialogs/dlg_connect.cpp
    src/interface/widgets/dialogs/dlg_convert_deck_to_cod_format.cpp
    src/interface/widgets/dialogs/dlg_create_game.cpp
    src/interface/widgets/dialogs/dlg_default_tags_editor.cpp
    src/interface/widgets/dialogs/dlg_edit_avatar.cpp
    src/interface/widgets/dialogs/dlg_edit_password.cpp
    src/interface/widgets/dialogs/dlg_edit_tokens.cpp
    src/interface/widgets/dialogs/dlg_edit_user.cpp
    src/interface/widgets/dialogs/dlg_filter_games.cpp
    src/interface/widgets/dialogs/dlg_forgot_password_challenge.cpp
    src/interface/widgets/dialogs/dlg_forgot_password_request.cpp
    src/interface/widgets/dialogs/dlg_forgot_password_reset.cpp
    src/interface/widgets/dialogs/dlg_load_deck.cpp
    src/interface/widgets/dialogs/dlg_load_deck_from_clipboard.cpp
    src/interface/widgets/dialogs/dlg_load_deck_from_website.cpp
    src/interface/widgets/dialogs/dlg_load_remote_deck.cpp
    src/interface/widgets/dialogs/dlg_manage_sets.cpp
    src/interface/widgets/dialogs/dlg_register.cpp
    src/interface/widgets/dialogs/dlg_select_set_for_cards.cpp
    src/interface/widgets/dialogs/dlg_settings.cpp
    src/interface/widgets/dialogs/dlg_startup_card_check.cpp
    src/interface/widgets/dialogs/dlg_tip_of_the_day.cpp
    src/interface/widgets/dialogs/dlg_update.cpp
    src/interface/widgets/dialogs/dlg_view_log.cpp
    src/interface/widgets/dialogs/tip_of_the_day.cpp
    src/filters/deck_filter_string.cpp
    src/filters/filter_builder.cpp
    src/filters/filter_tree_model.cpp
    src/filters/syntax_help.cpp
    src/game/abstract_game.cpp
    src/game/board/abstract_card_drag_item.cpp
    src/game/board/abstract_card_item.cpp
    src/game/board/abstract_counter.cpp
    src/game/board/arrow_item.cpp
    src/game/board/arrow_target.cpp
    src/game/board/card_drag_item.cpp
    src/game/board/card_item.cpp
    src/game/board/card_list.cpp
    src/game/board/counter_general.cpp
    src/game/board/translate_counter_name.cpp
    src/game/deckview/deck_view.cpp
    src/game/deckview/deck_view_container.cpp
    src/game/deckview/tabbed_deck_view_container.cpp
    src/game/dialogs/dlg_create_token.cpp
    src/game/dialogs/dlg_move_top_cards_until.cpp
    src/game/dialogs/dlg_roll_dice.cpp
    src/game/game.cpp
    src/game/game_event_handler.cpp
    src/game/game_meta_info.cpp
    src/game/game_scene.cpp
    src/game/game_state.cpp
    src/game/game_view.cpp
    src/game/hand_counter.cpp
    src/game/log/message_log_widget.cpp
    src/game/phase.cpp
    src/game/phases_toolbar.cpp
    src/game/player/menu/card_menu.cpp
    src/game/player/menu/custom_zone_menu.cpp
    src/game/player/menu/grave_menu.cpp
    src/game/player/menu/hand_menu.cpp
    src/game/player/menu/library_menu.cpp
    src/game/player/menu/move_menu.cpp
    src/game/player/menu/player_menu.cpp
    src/game/player/menu/pt_menu.cpp
    src/game/player/menu/rfg_menu.cpp
    src/game/player/menu/say_menu.cpp
    src/game/player/menu/sideboard_menu.cpp
    src/game/player/menu/utility_menu.cpp
    src/game/player/player.cpp
    src/game/player/player_actions.cpp
    src/game/player/player_area.cpp
    src/game/player/player_event_handler.cpp
    src/game/player/player_graphics_item.cpp
    src/game/player/player_info.cpp
    src/game/player/player_list_widget.cpp
    src/game/player/player_manager.cpp
    src/game/player/player_target.cpp
    src/game/replay.cpp
    src/game/zones/card_zone.cpp
    src/game/zones/hand_zone.cpp
    src/game/zones/logic/card_zone_logic.cpp
    src/game/zones/logic/hand_zone_logic.cpp
    src/game/zones/logic/pile_zone_logic.cpp
    src/game/zones/logic/stack_zone_logic.cpp
    src/game/zones/logic/table_zone_logic.cpp
    src/game/zones/logic/view_zone_logic.cpp
    src/game/zones/pile_zone.cpp
    src/game/zones/select_zone.cpp
    src/game/zones/stack_zone.cpp
    src/game/zones/table_zone.cpp
    src/game/zones/view_zone.cpp
    src/game/zones/view_zone_widget.cpp
    src/game_graphics/board/abstract_graphics_item.cpp
    src/interface/card_picture_loader/card_picture_loader.cpp
    src/interface/card_picture_loader/card_picture_loader_local.cpp
    src/interface/card_picture_loader/card_picture_loader_request_status_display_widget.cpp
    src/interface/card_picture_loader/card_picture_loader_status_bar.cpp
    src/interface/card_picture_loader/card_picture_loader_worker.cpp
    src/interface/card_picture_loader/card_picture_loader_worker_work.cpp
    src/interface/card_picture_loader/card_picture_to_load.cpp
    src/interface/layouts/flow_layout.cpp
    src/interface/layouts/overlap_layout.cpp
    src/interface/widgets/utility/line_edit_completer.cpp
    src/interface/pixel_map_generator.cpp
    src/interface/theme_manager.cpp
    src/interface/widgets/cards/additional_info/color_identity_widget.cpp
    src/interface/widgets/cards/additional_info/mana_cost_widget.cpp
    src/interface/widgets/cards/additional_info/mana_symbol_widget.cpp
    src/interface/widgets/cards/card_group_display_widgets/card_group_display_widget.cpp
    src/interface/widgets/cards/card_group_display_widgets/flat_card_group_display_widget.cpp
    src/interface/widgets/cards/card_group_display_widgets/overlapped_card_group_display_widget.cpp
    src/interface/widgets/cards/card_info_display_widget.cpp
    src/interface/widgets/cards/card_info_frame_widget.cpp
    src/interface/widgets/cards/card_info_picture_art_crop_widget.cpp
    src/interface/widgets/cards/card_info_picture_enlarged_widget.cpp
    src/interface/widgets/cards/card_info_picture_widget.cpp
    src/interface/widgets/cards/card_info_picture_with_text_overlay_widget.cpp
    src/interface/widgets/cards/card_info_text_widget.cpp
    src/interface/widgets/cards/card_size_widget.cpp
    src/interface/widgets/cards/deck_card_zone_display_widget.cpp
    src/interface/widgets/cards/deck_preview_card_picture_widget.cpp
    src/interface/widgets/deck_analytics/deck_analytics_widget.cpp
    src/interface/widgets/deck_analytics/mana_base_widget.cpp
    src/interface/widgets/deck_analytics/mana_curve_widget.cpp
    src/interface/widgets/deck_analytics/mana_devotion_widget.cpp
    src/interface/widgets/deck_editor/deck_list_history_manager_widget.cpp
    src/interface/widgets/deck_editor/deck_editor_card_info_dock_widget.cpp
    src/interface/widgets/deck_editor/deck_editor_database_display_widget.cpp
    src/interface/widgets/deck_editor/deck_editor_deck_dock_widget.cpp
    src/interface/widgets/deck_editor/deck_editor_filter_dock_widget.cpp
    src/interface/widgets/deck_editor/deck_editor_printing_selector_dock_widget.cpp
    src/interface/widgets/deck_editor/deck_list_style_proxy.cpp
    src/interface/widgets/general/background_sources.cpp
    src/interface/widgets/general/display/banner_widget.cpp
    src/interface/widgets/general/display/bar_widget.cpp
    src/interface/widgets/general/display/dynamic_font_size_label.cpp
    src/interface/widgets/general/display/dynamic_font_size_push_button.cpp
    src/interface/widgets/general/display/labeled_input.cpp
    src/interface/widgets/general/display/percent_bar_widget.cpp
    src/interface/widgets/general/display/shadow_background_label.cpp
    src/interface/widgets/general/home_styled_button.cpp
    src/interface/widgets/general/home_widget.cpp
    src/interface/widgets/general/layout_containers/flow_widget.cpp
    src/interface/widgets/general/layout_containers/overlap_control_widget.cpp
    src/interface/widgets/general/layout_containers/overlap_widget.cpp
    src/interface/widgets/menus/deck_editor_menu.cpp
    src/interface/widgets/printing_selector/all_zones_card_amount_widget.cpp
    src/interface/widgets/printing_selector/card_amount_widget.cpp
    src/interface/widgets/printing_selector/printing_selector.cpp
    src/interface/widgets/printing_selector/printing_selector_card_display_widget.cpp
    src/interface/widgets/printing_selector/printing_selector_card_overlay_widget.cpp
    src/interface/widgets/printing_selector/printing_selector_card_search_widget.cpp
    src/interface/widgets/printing_selector/printing_selector_card_selection_widget.cpp
    src/interface/widgets/printing_selector/printing_selector_card_sorting_widget.cpp
    src/interface/widgets/printing_selector/set_name_and_collectors_number_display_widget.cpp
    src/interface/widgets/quick_settings/settings_button_widget.cpp
    src/interface/widgets/quick_settings/settings_popup_widget.cpp
    src/interface/widgets/replay/replay_manager.cpp
    src/interface/widgets/replay/replay_timeline_widget.cpp
    src/interface/widgets/server/chat_view/chat_view.cpp
    src/interface/widgets/server/game_selector.cpp
    src/interface/widgets/server/game_selector_quick_filter_toolbar.cpp
    src/interface/widgets/server/games_model.cpp
    src/interface/widgets/server/handle_public_servers.cpp
    src/interface/widgets/server/remote/remote_decklist_tree_widget.cpp
    src/interface/widgets/server/remote/remote_replay_list_tree_widget.cpp
    src/interface/widgets/server/user/user_context_menu.cpp
    src/interface/widgets/server/user/user_info_box.cpp
    src/interface/widgets/server/user/user_info_connection.cpp
    src/interface/widgets/server/user/user_list_manager.cpp
    src/interface/widgets/server/user/user_list_widget.cpp
    src/interface/widgets/utility/custom_line_edit.cpp
    src/interface/widgets/utility/get_text_with_max.cpp
    src/interface/widgets/utility/sequence_edit.cpp
    src/interface/widgets/visual_database_display/visual_database_display_color_filter_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_filter_save_load_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_main_type_filter_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_name_filter_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_set_filter_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_sub_type_filter_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_display_widget.cpp
    src/interface/widgets/visual_database_display/visual_database_filter_display_widget.cpp
    src/interface/widgets/visual_deck_editor/visual_deck_editor_sample_hand_widget.cpp
    src/interface/widgets/visual_deck_editor/visual_deck_editor_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_color_identity_filter_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_deck_tags_display_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_tag_addition_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_tag_dialog.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_tag_display_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_tag_item_widget.cpp
    src/interface/widgets/visual_deck_storage/deck_preview/deck_preview_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_folder_display_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_quick_settings_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_search_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_sort_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_tag_filter_widget.cpp
    src/interface/widgets/visual_deck_storage/visual_deck_storage_widget.cpp
    src/interface/window_main.cpp
    src/main.cpp
    src/interface/widgets/tabs/abstract_tab_deck_editor.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/archidekt_links/edhrec_api_response_archidekt_links.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/average_deck/edhrec_average_deck_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/average_deck/edhrec_deck_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/card_prices/edhrec_api_response_card_prices.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/cards/edhrec_api_response_card_container.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/cards/edhrec_api_response_card_details.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/cards/edhrec_api_response_card_list.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/cards/edhrec_commander_api_response_commander_details.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/commander/edhrec_commander_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/commander/edhrec_commander_api_response_average_deck_statistics.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/top_cards/edhrec_top_cards_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/top_commanders/edhrec_top_commanders_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/api_response/top_tags/edhrec_top_tags_api_response.cpp
    src/interface/widgets/tabs/api/edhrec/display/card_prices/edhrec_api_response_card_prices_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/cards/edhrec_api_response_card_details_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/cards/edhrec_api_response_card_inclusion_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/cards/edhrec_api_response_card_list_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/cards/edhrec_api_response_card_synergy_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/commander/edhrec_api_response_commander_details_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/commander/edhrec_commander_api_response_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/commander/edhrec_commander_api_response_navigation_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/top_cards/edhrec_top_cards_api_response_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/top_commander/edhrec_top_commanders_api_response_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/display/top_tags/edhrec_top_tags_api_response_display_widget.cpp
    src/interface/widgets/tabs/api/edhrec/tab_edhrec.cpp
    src/interface/widgets/tabs/api/edhrec/tab_edhrec_main.cpp
    src/interface/widgets/tabs/tab.cpp
    src/interface/widgets/tabs/tab_account.cpp
    src/interface/widgets/tabs/tab_admin.cpp
    src/interface/widgets/tabs/tab_deck_editor.cpp
    src/interface/widgets/tabs/tab_deck_storage.cpp
    src/interface/widgets/tabs/tab_game.cpp
    src/interface/widgets/tabs/tab_home.cpp
    src/interface/widgets/tabs/tab_logs.cpp
    src/interface/widgets/tabs/tab_message.cpp
    src/interface/widgets/tabs/tab_replays.cpp
    src/interface/widgets/tabs/tab_room.cpp
    src/interface/widgets/tabs/tab_server.cpp
    src/interface/widgets/tabs/tab_supervisor.cpp
    src/interface/widgets/tabs/tab_visual_database_display.cpp
    src/interface/widgets/tabs/visual_deck_editor/tab_deck_editor_visual.cpp
    src/interface/widgets/tabs/visual_deck_editor/tab_deck_editor_visual_tab_widget.cpp
    src/interface/widgets/tabs/visual_deck_storage/tab_deck_storage_visual.cpp
    src/interface/key_signals.cpp
    src/interface/logger.cpp
)

add_subdirectory(sounds)
add_subdirectory(themes)
configure_file(
  ${CMAKE_SOURCE_DIR}/cockatrice/resources/config/qtlogging.ini ${CMAKE_BINARY_DIR}/cockatrice/qtlogging.ini COPYONLY
)

set(cockatrice_RESOURCES cockatrice.qrc)

if(UPDATE_TRANSLATIONS)
  # Cockatrice main sources
  file(GLOB_RECURSE translate_cockatrice_SRCS ${CMAKE_SOURCE_DIR}/cockatrice/src/*.cpp
       ${CMAKE_SOURCE_DIR}/cockatrice/src/*.h
  )

  # All libcockatrice_* libraries (recursively)
  file(GLOB_RECURSE translate_lib_SRCS ${CMAKE_SOURCE_DIR}/libcockatrice_*/**/*.cpp
       ${CMAKE_SOURCE_DIR}/libcockatrice_*/**/*.h
  )

  # Combine all sources for translation
  set(translate_SRCS ${translate_cockatrice_SRCS} ${translate_lib_SRCS})

  set(cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/cockatrice_en@source.ts")
else()
  file(GLOB cockatrice_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
endif()

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
  target_link_libraries(
    cockatrice
    libcockatrice_card
    libcockatrice_deck_list
    libcockatrice_filters
    libcockatrice_utility
    libcockatrice_network
    libcockatrice_models
    libcockatrice_rng
    libcockatrice_settings
    ${COCKATRICE_QT_MODULES}
  )
else()
  target_link_libraries(
    cockatrice
    PUBLIC libcockatrice_card
           libcockatrice_deck_list
           libcockatrice_filters
           libcockatrice_utility
           libcockatrice_network
           libcockatrice_models
           libcockatrice_rng
           libcockatrice_settings
           ${COCKATRICE_QT_MODULES}
  )
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

  # Qt plugins: audio (Qt5), iconengines, imageformats, multimedia (Qt6), platforms, printsupport (Qt5), styles, tls (Qt6)
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
    PATTERN "multimedia/*.dylib"
    PATTERN "platforms/*.dylib"
    PATTERN "printsupport/*.dylib"
    PATTERN "styles/*.dylib"
    PATTERN "tls/*.dylib"
  )

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/cockatrice/"
    DESTINATION ${qtconf_dest_dir}/
    FILES_MATCHING
    PATTERN "*.ini"
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

  install(
    DIRECTORY "${CMAKE_BINARY_DIR}/cockatrice/"
    DESTINATION ./
    FILES_MATCHING
    PATTERN "*.ini"
  )

  # Qt plugins: audio (Qt5), iconengines, imageformats, multimedia (Qt6) platforms, printsupport (Qt5), styles, tls (Qt6)
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
    PATTERN "multimedia/*.dll"
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
