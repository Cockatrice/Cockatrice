TEMPLATE = app
TARGET =
DEPENDPATH += . src
INCLUDEPATH += . src
MOC_DIR = build
OBJECTS_DIR = build
RESOURCES = cockatrice.qrc
QT += network svg
HEADERS += src/counter.h \
 src/gameselector.h \
 src/dlg_creategame.h \
 src/dlg_connect.h \
 src/gamesmodel.h \
 src/client.h \
 src/window_main.h \
 src/servergame.h \
 src/servereventdata.h \
 src/zonelist.h \
 src/cardzone.h \
 src/player.h \
 src/cardlist.h \
 src/carditem.h \
 src/tablezone.h \
 src/handzone.h \
 src/playerlist.h \
 src/game.h \
 src/carddatabase.h \
 src/gameview.h \
 src/decklistmodel.h \
 src/dlg_startgame.h \
 src/cardinfowidget.h \
 src/messagelogwidget.h \
 src/serverzonecard.h \
 src/zoneviewzone.h \
 src/zoneviewwidget.h \
 src/pilezone.h \
 src/carddragitem.h \
 src/zoneviewlayout.h \
 src/playerarea.h \
 src/carddatabasemodel.h \
 src/window_deckeditor.h \
 src/decklist.h \
 setsmodel.h \
 src/window_sets.h \
 src/abstractgraphicsitem.h \
 src/dlg_settings.h \
 src/phasestoolbar.h \
 src/phasebutton.h
SOURCES += src/counter.cpp \
 src/gameselector.cpp \
 src/dlg_creategame.cpp \
 src/dlg_connect.cpp \
 src/client.cpp \
 src/main.cpp \
 src/window_main.cpp \
 src/servereventdata.cpp \
 src/gamesmodel.cpp \
 src/player.cpp \
 src/cardzone.cpp \
 src/zonelist.cpp \
 src/cardlist.cpp \
 src/carditem.cpp \
 src/tablezone.cpp \
 src/handzone.cpp \
 src/playerlist.cpp \
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
 src/playerarea.cpp \
 src/carddatabasemodel.cpp \
 src/window_deckeditor.cpp \
 src/decklist.cpp \
 src/setsmodel.cpp \
 src/window_sets.cpp \
 src/abstractgraphicsitem.cpp \
 src/dlg_settings.cpp \
 src/phasestoolbar.cpp \
 src/phasebutton.cpp
TRANSLATIONS += translations/cockatrice_de.ts translations/cockatrice_en.ts
