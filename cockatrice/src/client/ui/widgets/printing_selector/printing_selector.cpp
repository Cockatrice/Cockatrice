#include "printing_selector.h"

#include "../../../../utility/card_set_comparator.h"
#include "printing_selector_card_display_widget.h"

#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QScrollBar>

const QString PrintingSelector::SORT_OPTIONS_ALPHABETICAL = tr("Alphabetical");
const QString PrintingSelector::SORT_OPTIONS_PREFERENCE = tr("Preference");
const QString PrintingSelector::SORT_OPTIONS_RELEASE_DATE = tr("Release Date");
const QString PrintingSelector::SORT_OPTIONS_CONTAINED_IN_DECK = tr("Contained in Deck");
const QString PrintingSelector::SORT_OPTIONS_POTENTIAL_CARDS = tr("Potential Cards in Deck");

const QStringList PrintingSelector::SORT_OPTIONS = {SORT_OPTIONS_ALPHABETICAL, SORT_OPTIONS_PREFERENCE,
                                                    SORT_OPTIONS_RELEASE_DATE, SORT_OPTIONS_CONTAINED_IN_DECK,
                                                    SORT_OPTIONS_POTENTIAL_CARDS};
PrintingSelector::PrintingSelector(QWidget *parent,
                                   TabDeckEditor *deckEditor,
                                   DeckListModel *deckModel,
                                   QTreeView *deckView)
    : QWidget(parent), deckEditor(deckEditor), deckModel(deckModel), deckView(deckView)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QVBoxLayout();
    setLayout(layout);
    timer = new QTimer(this);

    sortToolBar = new QHBoxLayout(this);

    sortOptionsSelector = new QComboBox(this);
    sortOptionsSelector->addItems(SORT_OPTIONS);
    sortOptionsSelector->setCurrentIndex(2);
    connect(sortOptionsSelector, &QComboBox::currentTextChanged, this, &PrintingSelector::updateDisplay);
    sortToolBar->addWidget(sortOptionsSelector);

    toggleSortOrder = new QPushButton(this);
    toggleSortOrder->setText(tr("Descending"));
    descendingSort = true;
    connect(toggleSortOrder, &QPushButton::clicked, this, &PrintingSelector::updateSortOrder);
    sortToolBar->addWidget(toggleSortOrder);

    layout->addLayout(sortToolBar);

    // Add the search bar
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(tr("Search by set name or set code"));
    layout->addWidget(searchBar);

    // Add a debounce timer for the search bar
    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    connect(searchBar, &QLineEdit::textChanged, this, [this]() {
        searchDebounceTimer->start(300); // 300ms debounce
    });
    connect(searchDebounceTimer, &QTimer::timeout, this, &PrintingSelector::updateDisplay);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);

    cardSizeWidget = new QWidget(this);
    cardSizeLayout = new QHBoxLayout(this);
    cardSizeWidget->setLayout(cardSizeLayout);

    cardSizeLabel = new QLabel(tr("Card Size"), cardSizeWidget);
    cardSizeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    cardSizeSlider = new QSlider(Qt::Horizontal, cardSizeWidget);
    cardSizeSlider->setRange(25, 250);
    cardSizeSlider->setValue(100);
    connect(cardSizeSlider, &QSlider::valueChanged, flowWidget, &FlowWidget::setMinimumSizeToMaxSizeHint);

    cardSizeLayout->addWidget(cardSizeLabel);
    cardSizeLayout->addWidget(cardSizeSlider);

    layout->addWidget(cardSizeWidget);

    cardSelectionBar = new QWidget(this);
    cardSelectionBarLayout = new QHBoxLayout(cardSelectionBar);
    previousCardButton = new QPushButton(cardSelectionBar);
    previousCardButton->setText(tr("Previous Card"));
    connect(previousCardButton, &QPushButton::clicked, this, &PrintingSelector::selectPreviousCard);
    nextCardButton = new QPushButton(cardSelectionBar);
    nextCardButton->setText(tr("Next Card"));
    connect(nextCardButton, &QPushButton::clicked, this, &PrintingSelector::selectNextCard);

    cardSelectionBarLayout->addWidget(previousCardButton);
    cardSelectionBarLayout->addWidget(nextCardButton);

    layout->addWidget(cardSelectionBar);
}

void PrintingSelector::updateSortOrder()
{
    if (descendingSort) {
        toggleSortOrder->setText(tr("Ascending"));
    } else {
        toggleSortOrder->setText(tr("Descending"));
    }
    descendingSort = !descendingSort;
    updateDisplay();
}

void PrintingSelector::updateDisplay()
{
    timer->stop();
    timer->deleteLater();
    timer = new QTimer(this);
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

    for (const auto &x : cardInfoPerSets) {
        for (const auto &cardInfoPerSet : x) {
            if (cardInfoPerSet.getProperty("uuid") == uuid) {
                return cardInfoPerSet;
            }
        }
    }

    return CardInfoPerSet();
}

QList<CardInfoPerSet> PrintingSelector::prependPrintingsInDeck(const QList<CardInfoPerSet> &sets)
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();
    QList<QPair<CardInfoPerSet, int>> countList;

    // Collect sets with their counts
    for (const auto &x : cardInfoPerSets) {
        for (const auto &cardInfoPerSet : x) {
            QModelIndex find_card =
                deckModel->findCard(selectedCard->getName(), DECK_ZONE_MAIN, cardInfoPerSet.getProperty("uuid"));
            if (find_card.isValid()) {
                int count =
                    deckModel->data(find_card, Qt::DisplayRole).toInt(); // Ensure the count is treated as an integer
                if (count > 0) {
                    countList.append(qMakePair(cardInfoPerSet, count));
                }
            }
            break;
        }
    }

    // Sort sets by count in descending numerical order
    std::sort(countList.begin(), countList.end(),
              [](const QPair<CardInfoPerSet, int> &a, const QPair<CardInfoPerSet, int> &b) {
                  return a.second > b.second; // Ensure numerical comparison
              });

    // Create a copy of the original list to modify
    QList<CardInfoPerSet> result = sets;

    // Prepend sorted sets and remove them from the original list
    for (const auto &pair : countList) {
        auto it = std::find_if(result.begin(), result.end(), [&pair](const CardInfoPerSet &item) {
            return item.getProperty("uuid") == pair.first.getProperty("uuid");
        });
        if (it != result.end()) {
            result.erase(it); // Remove the matching entry
        }
        result.prepend(pair.first); // Prepend the sorted item
    }

    return result;
}

QList<CardInfoPerSet> PrintingSelector::sortSets()
{
    if (selectedCard.isNull()) {
        return {};
    }
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();

    QList<CardSetPtr> sortedSets;

    for (const auto &x : cardInfoPerSets) {
        for (const auto &set : x) {
            sortedSets << set.getPtr();
            break;
        }
    }

    if (sortedSets.empty()) {
        sortedSets << CardSet::newInstance("", "", "", QDate());
    }

    if (sortOptionsSelector->currentText() == SORT_OPTIONS_PREFERENCE) {
        std::sort(sortedSets.begin(), sortedSets.end(), SetPriorityComparator());
    } else if (sortOptionsSelector->currentText() == SORT_OPTIONS_RELEASE_DATE) {
        std::sort(sortedSets.begin(), sortedSets.end(), SetReleaseDateComparator());
    }

    QList<CardInfoPerSet> sortedCardInfoPerSets;
    // Reconstruct sorted list of CardInfoPerSet
    for (const auto &set : sortedSets) {
        for (auto it = cardInfoPerSets.begin(); it != cardInfoPerSets.end(); ++it) {
            for (const auto &x : it.value()) {
                if (x.getPtr() == set) {
                    sortedCardInfoPerSets << it.value();
                    break;
                }
            }
        }
    }

    if (descendingSort) {
        std::reverse(sortedCardInfoPerSets.begin(), sortedCardInfoPerSets.end());
    }

    return sortedCardInfoPerSets;
}

QList<CardInfoPerSet> PrintingSelector::filterSets(const QList<CardInfoPerSet> &sets) const
{
    const QString searchText = searchBar->text().trimmed().toLower();

    if (searchText.isEmpty()) {
        return sets;
    }

    QList<CardInfoPerSet> filteredSets;

    for (const auto &set : sets) {
        const QString longName = set.getPtr()->getLongName().toLower();
        const QString shortName = set.getPtr()->getShortName().toLower();

        if (longName.contains(searchText) || shortName.contains(searchText)) {
            filteredSets << set;
        }
    }

    return filteredSets;
}

void PrintingSelector::getAllSetsForCurrentCard()
{
    const QList<CardInfoPerSet> sortedSets = sortSets();
    const QList<CardInfoPerSet> filteredSets = filterSets(sortedSets);
    const QList<CardInfoPerSet> prependedSets = prependPrintingsInDeck(filteredSets);

    // Defer widget creation
    currentIndex = 0;

    connect(timer, &QTimer::timeout, this, [=]() mutable {
        for (int i = 0; i < BATCH_SIZE && currentIndex < prependedSets.size(); ++i, ++currentIndex) {
            auto *cardDisplayWidget =
                new PrintingSelectorCardDisplayWidget(this, deckEditor, deckModel, deckView, cardSizeSlider,
                                                      selectedCard, prependedSets[currentIndex], currentZone);
            flowWidget->addWidget(cardDisplayWidget);
            cardDisplayWidget->clampSetNameToPicture();
        }

        // Stop timer when done
        if (currentIndex >= prependedSets.size()) {
            timer->stop();
        }
    });
    currentIndex = 0;
    timer->start(0); // Process as soon as possible
}
