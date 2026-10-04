/**
 * @file tabbed_deck_view_container.h
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef TABBED_DECK_VIEW_CONTAINER_H
#define TABBED_DECK_VIEW_CONTAINER_H
#include <QMap>
#include <QString>
#include <QTabWidget>
#include <qtmetamacros.h>

class DeckList;
class DeckView;
class DeckViewContainer;
class TabGame;

class TabbedDeckViewContainer : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabbedDeckViewContainer(int _playerId, TabGame *parent);
    void closeTab(int index);
    void updateTabBarVisibility();
    void addOpponentDeckView(const DeckList &opponentDeck, int opponentId, QString opponentName);
    int playerId;
    TabGame *parentGame;
    DeckViewContainer *playerDeckView;

    QMap<int, DeckView *> opponentDeckViews;
};

#endif // TABBED_DECK_VIEW_CONTAINER_H
