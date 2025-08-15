#ifndef TABBED_DECK_VIEW_CONTAINER_H
#define TABBED_DECK_VIEW_CONTAINER_H
#include "deck_view_container.h"

#include <QTabWidget>

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
