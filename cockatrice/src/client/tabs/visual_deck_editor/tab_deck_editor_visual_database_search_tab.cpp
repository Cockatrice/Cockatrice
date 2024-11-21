#include "tab_deck_editor_visual_database_search_tab.h"

#include "../../../game/cards/card_database_manager.h"
#include "../../../main.h"

TabDeckEditorVisualDatabaseSearchTab::TabDeckEditorVisualDatabaseSearchTab(TabSupervisor *_tabSupervisor,
                                                                           QWidget *_parent)
    : Tab(_tabSupervisor), parent(_parent)
{
    // Set up the layout and add tab widget
    layout = new QVBoxLayout(this);

    this->setLayout(layout);

    visualDeckView = new VisualDeckEditorWidget(this, this->deckModel);
    visualDeckView->setObjectName("visualDeckView");
    visualDeckView->updateDisplay();

    layout->addWidget(visualDeckView);
}
TabDeckEditorVisualDatabaseSearchTab::~TabDeckEditorVisualDatabaseSearchTab()
{
}

void TabDeckEditorVisualDatabaseSearchTab::updateCardInfoLeft(const QModelIndex &current, const QModelIndex &previous)
{
    (void)current;
    (void)previous;
}
void TabDeckEditorVisualDatabaseSearchTab::updateCardInfoRight(const QModelIndex &current, const QModelIndex &previous)
{
    (void)current;
    (void)previous;
}
void TabDeckEditorVisualDatabaseSearchTab::updateSearch(const QString &search)
{
    (void)search;
}
void TabDeckEditorVisualDatabaseSearchTab::databaseCustomMenu(QPoint point)
{
    (void)point;
}
void TabDeckEditorVisualDatabaseSearchTab::actClearFilterAll()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actClearFilterOne()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actSwapCard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actAddCard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actAddCardToSideboard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actRemoveCard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actIncrement()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actDecrement()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actDecrementCard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::actDecrementCardFromSideboard()
{
}
void TabDeckEditorVisualDatabaseSearchTab::copyDatabaseCellContents()
{
}
void TabDeckEditorVisualDatabaseSearchTab::filterViewCustomContextMenu(const QPoint &point)
{
    (void)point;
}
void TabDeckEditorVisualDatabaseSearchTab::filterRemove(QAction *action)
{
    (void)action;
}

void TabDeckEditorVisualDatabaseSearchTab::saveDbHeaderState()
{
}
void TabDeckEditorVisualDatabaseSearchTab::setSaveStatus(bool newStatus)
{
    (void)newStatus;
}
void TabDeckEditorVisualDatabaseSearchTab::showSearchSyntaxHelp()
{
}
CardInfoPtr TabDeckEditorVisualDatabaseSearchTab::currentCardInfo() const
{
    const QModelIndex currentIndex = databaseView->selectionModel()->currentIndex();
    if (!currentIndex.isValid()) {
        return {};
    }

    const QString cardName = currentIndex.sibling(currentIndex.row(), 0).data().toString();

    return CardDatabaseManager::getInstance()->getCard(cardName);
}
void TabDeckEditorVisualDatabaseSearchTab::addCardHelper(QString zoneName)
{
    (void)zoneName;
}
void TabDeckEditorVisualDatabaseSearchTab::offsetCountAtIndex(const QModelIndex &idx, int offset)
{
    (void)idx;
    (void)offset;
}
void TabDeckEditorVisualDatabaseSearchTab::decrementCardHelper(QString zoneName)
{
    (void)zoneName;
}
void TabDeckEditorVisualDatabaseSearchTab::recursiveExpand(const QModelIndex &index)
{
    (void)index;
}

void TabDeckEditorVisualDatabaseSearchTab::retranslateUi()
{
}
void TabDeckEditorVisualDatabaseSearchTab::setDeck(DeckLoader *_deckLoader)
{
    (void)_deckLoader;
}
void TabDeckEditorVisualDatabaseSearchTab::setModified(bool _windowModified)
{
    (void)_windowModified;
}
bool TabDeckEditorVisualDatabaseSearchTab::confirmClose()
{
    return true;
}
