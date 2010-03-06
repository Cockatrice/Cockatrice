TEMPLATE = app
TARGET =
DEPENDPATH += . src ../common
INCLUDEPATH += . src ../common
MOC_DIR = build
OBJECTS_DIR = build
RESOURCES = cockatrice.qrc
QT += network svg

HEADERS += src/counter.h \
 src/dlg_creategame.h \
 src/dlg_connect.h \
 src/gamesmodel.h \
 src/client.h \
 src/window_main.h \
 src/cardzone.h \
 src/player.h \
 src/cardlist.h \
 src/abstractcarditem.h \
 src/carditem.h \
 src/tablezone.h \
 src/handzone.h \
 src/carddatabase.h \
 src/gameview.h \
 src/deck_picturecacher.h \
 src/decklistmodel.h \
 src/dlg_load_remote_deck.h \
 src/cardinfowidget.h \
 src/messagelogwidget.h \
 src/zoneviewzone.h \
 src/zoneviewwidget.h \
 src/pilezone.h \
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
 src/tab.h \
 src/tab_server.h \
 src/tab_chatchannel.h \
 src/tab_game.h \
 src/tab_deck_storage.h \
 src/tab_supervisor.h \
 src/remotedecklist_treewidget.h \
 src/deckview.h \
 src/playerlistwidget.h \
 src/pingpixmapgenerator.h \
 src/settingscache.h \
 	../common/serializable_item.h \
	../common/decklist.h \
	../common/protocol.h \
	../common/protocol_items.h \
	../common/protocol_datastructures.h

SOURCES += src/counter.cpp \
 src/dlg_creategame.cpp \
 src/dlg_connect.cpp \
 src/client.cpp \
 src/main.cpp \
 src/window_main.cpp \
 src/gamesmodel.cpp \
 src/player.cpp \
 src/cardzone.cpp \
 src/cardlist.cpp \
 src/abstractcarditem.cpp \
 src/carditem.cpp \
 src/tablezone.cpp \
 src/handzone.cpp \
 src/carddatabase.cpp \
 src/gameview.cpp \
 src/deck_picturecacher.cpp \
 src/decklistmodel.cpp \
 src/dlg_load_remote_deck.cpp \
 src/cardinfowidget.cpp \
 src/messagelogwidget.cpp \
 src/zoneviewzone.cpp \
 src/zoneviewwidget.cpp \
 src/pilezone.cpp \
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
 src/tab_server.cpp \
 src/tab_chatchannel.cpp \
 src/tab_game.cpp \
 src/tab_deck_storage.cpp \
 src/tab_supervisor.cpp \
 src/remotedecklist_treewidget.cpp \
 src/deckview.cpp \
 src/playerlistwidget.cpp \
 src/pingpixmapgenerator.cpp \
 src/settingscache.cpp \
 	../common/serializable_item.cpp \
	../common/decklist.cpp \
	../common/protocol.cpp \
	../common/protocol_items.cpp \
	../common/protocol_datastructures.cpp
 
TRANSLATIONS += translations/cockatrice_de.ts translations/cockatrice_en.ts
CONFIG += qt debug
