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
 src/game.h \
 src/carddatabase.h \
 src/gameview.h \
 src/decklistmodel.h \
 src/dlg_startgame.h \
 src/cardinfowidget.h \
 src/messagelogwidget.h \
 src/zoneviewzone.h \
 src/zoneviewwidget.h \
 src/pilezone.h \
 src/carddragitem.h \
 src/zoneviewlayout.h \
 src/carddatabasemodel.h \
 src/window_deckeditor.h \
 src/setsmodel.h \
 src/window_sets.h \
 src/abstractgraphicsitem.h \
 src/dlg_settings.h \
 src/phasestoolbar.h \
 src/gamescene.h \
 src/arrowitem.h \
 src/tab_server.h \
 src/tab_chatchannel.h \
 src/tab_game.h \
 src/tab_deck_storage.h \
 src/tab_supervisor.h \
 src/deckview.h \
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
 src/game.cpp \
 src/carddatabase.cpp \
 src/gameview.cpp \
 src/decklistmodel.cpp \
 src/dlg_startgame.cpp \
 src/cardinfowidget.cpp \
 src/messagelogwidget.cpp \
 src/zoneviewzone.cpp \
 src/zoneviewwidget.cpp \
 src/pilezone.cpp \
 src/carddragitem.cpp \
 src/zoneviewlayout.cpp \
 src/carddatabasemodel.cpp \
 src/window_deckeditor.cpp \
 src/setsmodel.cpp \
 src/window_sets.cpp \
 src/abstractgraphicsitem.cpp \
 src/dlg_settings.cpp \
 src/phasestoolbar.cpp \
 src/gamescene.cpp \
 src/arrowitem.cpp \
 src/tab_server.cpp \
 src/tab_chatchannel.cpp \
 src/tab_game.cpp \
 src/tab_deck_storage.cpp \
 src/tab_supervisor.cpp \
 src/deckview.cpp \
 	../common/decklist.cpp \
	../common/protocol.cpp \
	../common/protocol_items.cpp
 
TRANSLATIONS += translations/cockatrice_de.ts translations/cockatrice_en.ts
CONFIG += qt debug
