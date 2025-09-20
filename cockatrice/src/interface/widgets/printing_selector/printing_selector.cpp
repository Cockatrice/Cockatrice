#include "printing_selector.h"

#include "../../../../dialogs/dlg_select_set_for_cards.h"
#include "../../../../settings/cache_settings.h"
#include "../../picture_loader/picture_loader.h"
#include "printing_selector_card_display_widget.h"
#include "printing_selector_card_search_widget.h"
#include "printing_selector_card_selection_widget.h"
#include "printing_selector_card_sorting_widget.h"

#include <QFrame>
#include <QScrollBar>
#include <qboxlayout.h>

/**
 * @brief Constructs a PrintingSelector widget to display and manage card printings.
 *
 * This constructor initializes the PrintingSelector widget, setting up various child widgets
 * such as sorting tools, search bar, card size options, and navigation controls. It also connects
 * signals and slots to update the display when the deck model changes, and loads available printings
 * for the selected card.
 *
 * @param parent The parent widget for the PrintingSelector.
 * @param deckEditor The TabDeckEditor instance used for managing the deck.
 * @param deckModel The DeckListModel instance that provides data for the deck's contents.
 * @param deckView The QTreeView instance used to display the deck and its contents.
 */
PrintingSelector::PrintingSelector(QWidget *parent, AbstractTabDeckEditor *_deckEditor)
    : QWidget(parent), deckEditor(_deckEditor), deckModel(deckEditor->deckDockWidget->deckModel),
      deckView(deckEditor->deckDockWidget->deckView)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QVBoxLayout(this);
    setLayout(layout);

    widgetLoadingBufferTimer = new QTimer(this);

    flowWidget = new FlowWidget(this, Qt::Horizontal, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);

    sortToolBar = new PrintingSelectorCardSortingWidget(this);
    sortToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    displayOptionsWidget = new SettingsButtonWidget(this);
    displayOptionsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // Create the checkbox for navigation buttons visibility
    navigationCheckBox = new QCheckBox(this);
    navigationCheckBox->setChecked(SettingsCache::instance().getPrintingSelectorNavigationButtonsVisible());
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PrintingSelector::toggleVisibilityNavigationButtons);
    connect(navigationCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorNavigationButtonsVisible);

    cardSizeWidget =
        new CardSizeWidget(displayOptionsWidget, flowWidget, SettingsCache::instance().getPrintingSelectorCardSize());
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance(),
            &SettingsCache::setPrintingSelectorCardSize);

    displayOptionsWidget->addSettingsWidget(sortToolBar);
    displayOptionsWidget->addSettingsWidget(navigationCheckBox);
    displayOptionsWidget->addSettingsWidget(cardSizeWidget);

    sortAndOptionsContainer = new QWidget(this);
    sortAndOptionsLayout = new QHBoxLayout(sortAndOptionsContainer);
    sortAndOptionsLayout->setSpacing(3);
    sortAndOptionsLayout->setContentsMargins(0, 0, 0, 0);
    sortAndOptionsContainer->setLayout(sortAndOptionsLayout);

    searchBar = new PrintingSelectorCardSearchWidget(this);

    sortAndOptionsLayout->addWidget(searchBar);
    sortAndOptionsLayout->addWidget(displayOptionsWidget);

    layout->addWidget(sortAndOptionsContainer);

    layout->addWidget(flowWidget);

    cardSelectionBar = new PrintingSelectorCardSelectionWidget(this);
    cardSelectionBar->setVisible(SettingsCache::instance().getPrintingSelectorNavigationButtonsVisible());
    layout->addWidget(cardSelectionBar);

    // Connect deck model data change signal to update display
    connect(deckModel, &DeckListModel::rowsInserted, this, &PrintingSelector::printingsInDeckChanged);
    connect(deckModel, &DeckListModel::rowsRemoved, this, &PrintingSelector::printingsInDeckChanged);

    retranslateUi();
}

void PrintingSelector::retranslateUi()
{
    navigationCheckBox->setText(tr("Display Navigation Buttons"));
}

void PrintingSelector::printingsInDeckChanged()
{
    // Delay the update to avoid race conditions
    QTimer::singleShot(100, this, &PrintingSelector::updateDisplay);
}

/**
 * @brief Updates the display by clearing the layout and loading new sets for the current card.
 */
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

/**
 * @brief Sets the current card for the selector and updates the display.
 *
 * @param newCard The new card to set.
 * @param _currentZone The current zone the card is in.
 */
void PrintingSelector::setCard(const CardInfoPtr &newCard, const QString &_currentZone)
{
    if (newCard.isNull()) {
        return;
    }

    // we don't need to redraw the widget if the card is the same
    if (!selectedCard.isNull() && selectedCard->getName() == newCard->getName()) {
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

/**
 * @brief Selects the previous card in the list.
 */
void PrintingSelector::selectPreviousCard()
{
    selectCard(-1);
}

/**
 * @brief Selects the next card in the list.
 */
void PrintingSelector::selectNextCard()
{
    selectCard(1);
}

/**
 * @brief Selects a card based on the change direction.
 *
 * @param changeBy The direction to change, -1 for previous, 1 for next.
 */
void PrintingSelector::selectCard(const int changeBy)
{
    if (changeBy == 0) {
        return;
    }

    // Get the current index of the selected item
    auto deckViewCurrentIndex = deckView->currentIndex();

    auto nextIndex = deckViewCurrentIndex.siblingAtRow(deckViewCurrentIndex.row() + changeBy);
    if (!nextIndex.isValid()) {
        nextIndex = deckViewCurrentIndex;

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

/**
 * @brief Loads and displays all sets for the current selected card.
 */
void PrintingSelector::getAllSetsForCurrentCard()
{
    if (selectedCard.isNull()) {
        return;
    }

    SetToPrintingsMap setMap = selectedCard->getSets();
    const QList<PrintingInfo> sortedPrintings = sortToolBar->sortSets(setMap);
    const QList<PrintingInfo> filteredPrintings =
        sortToolBar->filterSets(sortedPrintings, searchBar->getSearchText().trimmed().toLower());
    QList<PrintingInfo> printingsToUse;

    if (SettingsCache::instance().getBumpSetsWithCardsInDeckToTop()) {
        printingsToUse = sortToolBar->prependPrintingsInDeck(filteredPrintings, selectedCard, deckModel);
    } else {
        printingsToUse = filteredPrintings;
    }
    printingsToUse = sortToolBar->prependPinnedPrintings(printingsToUse, selectedCard->getName());

    // Defer widget creation
    currentIndex = 0;

    connect(widgetLoadingBufferTimer, &QTimer::timeout, this, [=, this]() mutable {
        for (int i = 0; i < BATCH_SIZE && currentIndex < printingsToUse.size(); ++i, ++currentIndex) {
            ExactCard card = ExactCard(selectedCard, printingsToUse[currentIndex]);
            auto *cardDisplayWidget = new PrintingSelectorCardDisplayWidget(
                this, deckEditor, deckModel, deckView, cardSizeWidget->getSlider(), card, currentZone);
            flowWidget->addWidget(cardDisplayWidget);
            cardDisplayWidget->clampSetNameToPicture();
            connect(cardDisplayWidget, &PrintingSelectorCardDisplayWidget::cardPreferenceChanged, this,
                    &PrintingSelector::updateDisplay);
        }

        // Stop timer when done
        if (currentIndex >= printingsToUse.size()) {
            widgetLoadingBufferTimer->stop();
        }
    });
    currentIndex = 0;
    widgetLoadingBufferTimer->start(0); // Process as soon as possible
}

/**
 * @brief Toggles the visibility of the navigation buttons.
 *
 * @param _state The visibility state to set.
 */
void PrintingSelector::toggleVisibilityNavigationButtons(bool _state)
{
    cardSelectionBar->setVisible(_state);
}
