TEMPLATE = app
TARGET =
DEPENDPATH += . src ../common
INCLUDEPATH += . src ../common
MOC_DIR = build
OBJECTS_DIR = build
RESOURCES = cockatrice.qrc
QT += network svg

HEADERS += src/abstractcounter.h \
 src/counter_general.h \
 src/dlg_creategame.h \
 src/dlg_connect.h \
 src/dlg_create_token.h \
 src/gamesmodel.h \
 src/abstractclient.h \
 src/remoteclient.h \
 src/window_main.h \
 src/cardzone.h \
 src/selectzone.h \
 src/player.h \
 src/playertarget.h \
 src/cardlist.h \
 src/abstractcarditem.h \
 src/carditem.h \
 src/tablezone.h \
 src/handzone.h \
 src/handcounter.h \
 src/carddatabase.h \
 src/gameview.h \
 src/decklistmodel.h \
 src/dlg_load_deck_from_clipboard.h \
 src/dlg_load_remote_deck.h \
 src/cardinfowidget.h \
 src/messagelogwidget.h \
 src/zoneviewzone.h \
 src/zoneviewwidget.h \
 src/pilezone.h \
 src/stackzone.h \
 src/carddragitem.h \
 src/carddatabasemodel.h \
 src/window_deckeditor.h \
 src/setsmodel.h \
 src/window_sets.h \
 src/abstractgraphicsitem.h \
 src/abstractcarddragitem.h \
 src/dlg_settings.h \
 src/dlg_cardsearch.h \
 src/phasestoolbar.h \
 src/gamescene.h \
 src/arrowitem.h \
 src/arrowtarget.h \
 src/tab.h \
 src/tab_server.h \
 src/tab_room.h \
 src/tab_message.h \
 src/tab_game.h \
 src/tab_deck_storage.h \
 src/tab_supervisor.h \
 src/tab_admin.h \
 src/userlist.h \
 src/userinfobox.h \
 src/remotedecklist_treewidget.h \
 src/deckview.h \
 src/playerlistwidget.h \
 src/pixmapgenerator.h \
 src/settingscache.h \
 src/localserver.h \
 src/localserverinterface.h \
 src/localclient.h \
 src/translation.h \
 	../common/color.h \
 	../common/serializable_item.h \
	../common/decklist.h \
	../common/protocol.h \
	../common/protocol_items.h \
	../common/protocol_datastructures.h \
	../common/rng_abstract.h \
	../common/rng_sfmt.h \
	../common/server.h \
	../common/server_arrow.h \
	../common/server_card.h \
	../common/server_cardzone.h \
	../common/server_room.h \
	../common/server_counter.h \
	../common/server_game.h \
	../common/server_player.h \
	../common/server_protocolhandler.h \
	../common/server_arrowtarget.h

SOURCES += src/abstractcounter.cpp \
 src/counter_general.cpp \
 src/dlg_creategame.cpp \
 src/dlg_connect.cpp \
 src/dlg_create_token.cpp \
 src/abstractclient.cpp \
 src/remoteclient.cpp \
 src/main.cpp \
 src/window_main.cpp \
 src/gamesmodel.cpp \
 src/player.cpp \
 src/playertarget.cpp \
 src/cardzone.cpp \
 src/selectzone.cpp \
 src/cardlist.cpp \
 src/abstractcarditem.cpp \
 src/carditem.cpp \
 src/tablezone.cpp \
 src/handzone.cpp \
 src/handcounter.cpp \
 src/carddatabase.cpp \
 src/gameview.cpp \
 src/decklistmodel.cpp \
 src/dlg_load_deck_from_clipboard.cpp \
 src/dlg_load_remote_deck.cpp \
 src/cardinfowidget.cpp \
 src/messagelogwidget.cpp \
 src/zoneviewzone.cpp \
 src/zoneviewwidget.cpp \
 src/pilezone.cpp \
 src/stackzone.cpp \
 src/carddragitem.cpp \
 src/carddatabasemodel.cpp \
 src/window_deckeditor.cpp \
 src/setsmodel.cpp \
 src/window_sets.cpp \
 src/abstractgraphicsitem.cpp \
 src/abstractcarddragitem.cpp \
 src/dlg_settings.cpp \
 src/dlg_cardsearch.cpp \
 src/phasestoolbar.cpp \
 src/gamescene.cpp \
 src/arrowitem.cpp \
 src/arrowtarget.cpp \
 src/tab_server.cpp \
 src/tab_room.cpp \
 src/tab_message.cpp \
 src/tab_game.cpp \
 src/tab_deck_storage.cpp \
 src/tab_supervisor.cpp \
 src/tab_admin.cpp \
 src/userlist.cpp \
 src/userinfobox.cpp \
 src/remotedecklist_treewidget.cpp \
 src/deckview.cpp \
 src/playerlistwidget.cpp \
 src/pixmapgenerator.cpp \
 src/settingscache.cpp \
 src/localserver.cpp \
 src/localserverinterface.cpp \
 src/localclient.cpp \
 	../common/serializable_item.cpp \
	../common/decklist.cpp \
	../common/protocol.cpp \
	../common/protocol_items.cpp \
	../common/protocol_datastructures.cpp \
	../common/rng_abstract.cpp \
	../common/rng_sfmt.cpp \
	../common/sfmt/SFMT.c \
	../common/server.cpp \
	../common/server_card.cpp \
	../common/server_cardzone.cpp \
	../common/server_room.cpp \
	../common/server_game.cpp \
	../common/server_player.cpp \
	../common/server_protocolhandler.cpp

TRANSLATIONS += \
	translations/cockatrice_de.ts \
	translations/cockatrice_en.ts \
	translations/cockatrice_es.ts \
	translations/cockatrice_pt.ts \
	translations/cockatrice_pt-br.ts \
	translations/cockatrice_fr.ts \
	translations/cockatrice_jp.ts
win32 {
	RC_FILE = cockatrice.rc
}
macx {
	ICON = resources/appicon.icns
	CONFIG += x86 ppc
	LIBS += -bind_at_load
}
