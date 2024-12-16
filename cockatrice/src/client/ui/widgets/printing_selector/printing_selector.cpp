#include "printing_selector.h"

#include "../../../../settings/cache_settings.h"
#include "printing_selector_card_display_widget.h"
#include "printing_selector_card_search_widget.h"
#include "printing_selector_card_selection_widget.h"
#include "printing_selector_card_sorting_widget.h"
#include "printing_selector_view_options_toolbar_widget.h"

#include <QScrollBar>

PrintingSelector::PrintingSelector(QWidget *parent,
                                   TabDeckEditor *deckEditor,
                                   DeckListModel *deckModel,
                                   QTreeView *deckView)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QVBoxLayout();
    setLayout(layout);
    widgetLoadingBufferTimer = new QTimer(this);

    viewOptionsToolbar = new PrintingSelectorViewOptionsToolbarWidget(this, this);
    layout->addWidget(viewOptionsToolbar);

    sortToolBar = new PrintingSelectorCardSortingWidget(this);
    layout->addWidget(sortToolBar);

    // Add the search bar
    searchBar = new PrintingSelectorCardSearchWidget(this);
    layout->addWidget(searchBar);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new CardSizeWidget(this, flowWidget, SettingsCache::instance().getPrintingSelectorCardSize());
    layout->addWidget(cardSizeWidget);

    cardSelectionBar = new PrintingSelectorCardSelectionWidget(this);
    layout->addWidget(cardSelectionBar);

    connect(deckModel, &DeckListModel::dataChanged, this, [this]() {
        // We have to delay this or else we hit a race condition where the data isn't actually updated yet.
        QTimer::singleShot(100, this, &PrintingSelector::updateDisplay);
    });
}

void PrintingSelector::updateDisplay()
{
    widgetLoadingBufferTimer->stop();
    widgetLoadingBufferTimer->deleteLater();
    widgetLoadingBufferTimer = new QTimer(this);
    flowWidget->clearLayout();
    if (selectedCard != nullptr) {
        setWindowTitle(selectedCard->getName());
    }
    getAllSetsForCurrentCard();
}

void PrintingSelector::setCard(const CardInfoPtr &newCard, const QString &_currentZone)
{
    if (newCard.isNull()) {
        return;
    }
    selectedCard = newCard;
    currentZone = _currentZone;
    if (isVisible()) {
        updateDisplay();
    }
    flowWidget->setMinimumSizeToMaxSizeHint();
    flowWidget->scrollArea->verticalScrollBar()->setValue(0);
    flowWidget->repaint();
}

void PrintingSelector::selectPreviousCard()
{
    selectCard(-1);
}

void PrintingSelector::selectNextCard()
{
    selectCard(1);
}

void PrintingSelector::selectCard(int changeBy)
{
    if (changeBy == 0) {
        return;
    }

    // Get the current index of the selected item
    auto currentIndex = deckView->currentIndex();

    auto nextIndex = currentIndex.siblingAtRow(currentIndex.row() + changeBy);
    if (!nextIndex.isValid()) {
        nextIndex = currentIndex;

        // Increment to the next valid index, skipping header rows
        AbstractDecklistNode *node;
        do {
            if (changeBy > 0) {
                nextIndex = deckView->indexBelow(nextIndex);
            } else {
                nextIndex = deckView->indexAbove(nextIndex);
            }
            node = static_cast<AbstractDecklistNode *>(nextIndex.internalPointer());
        } while (node && node->isDeckHeader());
    }

    if (nextIndex.isValid()) {
        deckView->setCurrentIndex(nextIndex);
        deckView->setFocus(Qt::FocusReason::MouseFocusReason);
    }
}

CardInfoPerSet PrintingSelector::getSetForUUID(const QString &uuid)
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();

    for (const auto &cardInfoPerSetList : cardInfoPerSets) {
        for (const auto &cardInfoPerSet : cardInfoPerSetList) {
            if (cardInfoPerSet.getProperty("uuid") == uuid) {
                return cardInfoPerSet;
            }
        }
    }

    return CardInfoPerSet();
}

void PrintingSelector::getAllSetsForCurrentCard()
{
    if (selectedCard.isNull()) {
        return;
    }

    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();
    const QList<CardInfoPerSet> sortedSets = sortToolBar->sortSets(cardInfoPerSets);
    const QList<CardInfoPerSet> filteredSets =
        sortToolBar->filterSets(sortedSets, searchBar->getSearchText().trimmed().toLower());
    QList<CardInfoPerSet> setsToUse;

    if (SettingsCache::instance().getBumpSetsWithCardsInDeckToTop()) {
        setsToUse = sortToolBar->prependPrintingsInDeck(filteredSets, selectedCard, deckModel);
    } else {
        setsToUse = filteredSets;
    }

    // Defer widget creation
    currentIndex = 0;

    connect(widgetLoadingBufferTimer, &QTimer::timeout, this, [=]() mutable {
        for (int i = 0; i < BATCH_SIZE && currentIndex < setsToUse.size(); ++i, ++currentIndex) {
            auto *cardDisplayWidget = new PrintingSelectorCardDisplayWidget(this, deckEditor, deckModel, deckView,
                                                                            cardSizeWidget->getSlider(), selectedCard,
                                                                            setsToUse[currentIndex], currentZone);
            flowWidget->addWidget(cardDisplayWidget);
            cardDisplayWidget->clampSetNameToPicture();
        }

        // Stop timer when done
        if (currentIndex >= setsToUse.size()) {
            widgetLoadingBufferTimer->stop();
        }
    });
    currentIndex = 0;
    widgetLoadingBufferTimer->start(0); // Process as soon as possible
}
