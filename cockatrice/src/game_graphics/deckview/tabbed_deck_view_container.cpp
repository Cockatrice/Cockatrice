#include "tabbed_deck_view_container.h"

#include "../../interface/widgets/tabs/tab_game.h"
#include "deck_view.h"

TabbedDeckViewContainer::TabbedDeckViewContainer(int _playerId, TabGame *parent)
    : QTabWidget(nullptr), playerId(_playerId), parentGame(parent)
{
    setTabsClosable(true);
    connect(this, &QTabWidget::tabCloseRequested, this, &TabbedDeckViewContainer::closeTab);

    playerDeckView = new DeckViewContainer(playerId, parentGame);
    int playerTabIndex = addTab(playerDeckView, "Your Deck");
    tabBar()->setTabButton(playerTabIndex, QTabBar::RightSide, nullptr);
    updateTabBarVisibility();
}

void TabbedDeckViewContainer::addOpponentDeckView(const DeckList &opponentDeck, int opponentId, QString opponentName)
{
    if (opponentDeckViews.contains(opponentId)) {
        opponentDeckViews[opponentId]->setDeck(opponentDeck);
    } else {
        auto *opponentDeckView = new DeckView(this);
        connect(opponentDeckView, &DeckView::newCardAdded, playerDeckView, &DeckViewContainer::newCardAdded);

        opponentDeckView->setDeck(opponentDeck);

        addTab(opponentDeckView, QString("%1's Deck").arg(opponentName));

        opponentDeckViews.insert(opponentId, opponentDeckView);
    }
    updateTabBarVisibility();
}

void TabbedDeckViewContainer::closeTab(int index)
{
    QWidget *widgetToClose = widget(index);

    // Prevent removing the player tab
    if (widgetToClose == playerDeckView) {
        return;
    }

    // Remove it from map if it's an opponent
    auto it = opponentDeckViews.begin();
    while (it != opponentDeckViews.end()) {
        if (it.value() == widgetToClose) {
            it = opponentDeckViews.erase(it);
        } else {
            ++it;
        }
    }

    removeTab(index);
    widgetToClose->deleteLater();
    updateTabBarVisibility();
}

void TabbedDeckViewContainer::updateTabBarVisibility()
{
    if (tabBar()->count() <= 1) {
        tabBar()->hide();
    } else {
        tabBar()->show();
    }
}
