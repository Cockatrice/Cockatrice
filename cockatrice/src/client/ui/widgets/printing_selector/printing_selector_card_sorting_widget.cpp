#include "printing_selector_card_sorting_widget.h"

#include "../../../../utility/card_set_comparator.h"

const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_ALPHABETICAL = tr("Alphabetical");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_PREFERENCE = tr("Preference");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_RELEASE_DATE = tr("Release Date");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_CONTAINED_IN_DECK = tr("Contained in Deck");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_POTENTIAL_CARDS = tr("Potential Cards in Deck");

const QStringList PrintingSelectorCardSortingWidget::SORT_OPTIONS = {
    SORT_OPTIONS_ALPHABETICAL, SORT_OPTIONS_PREFERENCE, SORT_OPTIONS_RELEASE_DATE, SORT_OPTIONS_CONTAINED_IN_DECK,
    SORT_OPTIONS_POTENTIAL_CARDS};

PrintingSelectorCardSortingWidget::PrintingSelectorCardSortingWidget(PrintingSelector *parent) : parent(parent)
{
    sortToolBar = new QHBoxLayout(this);

    sortOptionsSelector = new QComboBox(this);
    sortOptionsSelector->addItems(SORT_OPTIONS);
    sortOptionsSelector->setCurrentIndex(2);
    connect(sortOptionsSelector, &QComboBox::currentTextChanged, parent, &PrintingSelector::updateDisplay);
    sortToolBar->addWidget(sortOptionsSelector);

    toggleSortOrder = new QPushButton(this);
    toggleSortOrder->setText(tr("Descending"));
    descendingSort = true;
    connect(toggleSortOrder, &QPushButton::clicked, this, &PrintingSelectorCardSortingWidget::updateSortOrder);
    sortToolBar->addWidget(toggleSortOrder);
}

void PrintingSelectorCardSortingWidget::updateSortOrder()
{
    if (descendingSort) {
        toggleSortOrder->setText(tr("Ascending"));
    } else {
        toggleSortOrder->setText(tr("Descending"));
    }
    descendingSort = !descendingSort;
    parent->updateDisplay();
}

QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::sortSets(CardInfoPerSetMap cardInfoPerSets)
{
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
        std::reverse(sortedSets.begin(), sortedSets.end());
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

QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::filterSets(const QList<CardInfoPerSet> &sets,
                                                                    const QString searchText) const
{
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

QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::prependPrintingsInDeck(const QList<CardInfoPerSet> &sets,
                                                                                CardInfoPtr selectedCard,
                                                                                DeckListModel *deckModel)
{
    if (!selectedCard) {
        return {};
    }

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