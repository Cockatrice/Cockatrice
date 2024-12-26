#include "printing_selector_card_sorting_widget.h"

#include "../../../../settings/cache_settings.h"
#include "../../../../utility/card_set_comparator.h"

const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_ALPHABETICAL = tr("Alphabetical");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_PREFERENCE = tr("Preference");
const QString PrintingSelectorCardSortingWidget::SORT_OPTIONS_RELEASE_DATE = tr("Release Date");

const QStringList PrintingSelectorCardSortingWidget::SORT_OPTIONS = {SORT_OPTIONS_ALPHABETICAL, SORT_OPTIONS_PREFERENCE,
                                                                     SORT_OPTIONS_RELEASE_DATE};

/**
 * @brief A widget for sorting and filtering card sets in the Printing Selector.
 *
 * This widget allows users to choose sorting options for the card sets, such as alphabetical order, release date, or
 * user-defined preferences. It also allows users to toggle the sorting order between ascending and descending.
 */
PrintingSelectorCardSortingWidget::PrintingSelectorCardSortingWidget(PrintingSelector *parent) : parent(parent)
{
    sortToolBar = new QHBoxLayout(this);

    sortOptionsSelector = new QComboBox(this);
    sortOptionsSelector->addItems(SORT_OPTIONS);
    sortOptionsSelector->setCurrentIndex(SettingsCache::instance().getPrintingSelectorSortOrder());
    connect(sortOptionsSelector, &QComboBox::currentTextChanged, this,
            &PrintingSelectorCardSortingWidget::updateSortSetting);
    connect(sortOptionsSelector, &QComboBox::currentTextChanged, parent, &PrintingSelector::updateDisplay);
    sortToolBar->addWidget(sortOptionsSelector);

    toggleSortOrder = new QPushButton(this);
    toggleSortOrder->setText(tr("Descending"));
    descendingSort = true;
    connect(toggleSortOrder, &QPushButton::clicked, this, &PrintingSelectorCardSortingWidget::updateSortOrder);
    sortToolBar->addWidget(toggleSortOrder);
}

/**
 * @brief Updates the sorting order (ascending or descending).
 *
 * This function toggles the sort order between ascending and descending and updates the display.
 */
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

/**
 * @brief Updates the sorting setting in the application settings.
 *
 * This function saves the selected sorting option (from the combobox) to the application settings.
 */
void PrintingSelectorCardSortingWidget::updateSortSetting()
{
    SettingsCache::instance().setPrintingSelectorSortOrder(sortOptionsSelector->currentIndex());
}

/**
 * @brief Sorts a list of card sets based on the selected sorting option.
 *
 * This function sorts the card sets according to the selected sorting option in the combobox. The options include:
 * - Alphabetical
 * - Preference
 * - Release Date
 * - Contained in Deck
 * - Potential Cards in Deck
 *
 * @param cardInfoPerSets The list of card sets to be sorted.
 * @return A sorted list of card sets.
 */
QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::sortSets(const CardInfoPerSetMap &cardInfoPerSets)
{
    QList<CardSetPtr> sortedSets;

    for (const auto &cardInfoPerSetList : cardInfoPerSets) {
        for (const auto &set : cardInfoPerSetList) {
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
            for (const auto &cardInfoPerSet : it.value()) {
                if (cardInfoPerSet.getPtr() == set) {
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

/**
 * @brief Filters a list of card sets based on the search text.
 *
 * This function filters the given list of card sets by comparing their long and short names with the provided search
 * text. If the search text matches either the long or short name of a card set, that set is included in the filtered
 * list.
 *
 * @param sets The list of card sets to be filtered.
 * @param searchText The search text used to filter the card sets.
 * @return A filtered list of card sets.
 */
QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::filterSets(const QList<CardInfoPerSet> &sets,
                                                                    const QString &searchText)
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

QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::prependPinnedPrintings(const QList<CardInfoPerSet> &sets,
                                                                                  const QString &cardName)
{
    auto setsToUse = sets;
    const auto &cardProviderId = SettingsCache::instance().cardOverrides().getCardPreferenceOverride(cardName);
    if (!cardProviderId.isEmpty()) {
        for (int i = 0; i < setsToUse.size(); ++i) {
            const auto &card = setsToUse[i];
            if (card.getProperty("uuid") == cardProviderId) {
                setsToUse.move(i, 0);
                break;
            }
        }
    }

    return setsToUse;
}

/**
 * @brief Prepend card printings that are contained in the deck to the list of card sets.
 *
 * This function adjusts the list of card sets by moving the printings that are already contained in the deck to the
 * beginning of the list, sorted by the count of cards in the deck.
 *
 * @param sets The original list of card sets.
 * @param selectedCard The currently selected card.
 * @param deckModel The model representing the deck.
 * @return A list of card sets with the printings contained in the deck prepended.
 */
QList<CardInfoPerSet> PrintingSelectorCardSortingWidget::prependPrintingsInDeck(const QList<CardInfoPerSet> &sets,
                                                                                const CardInfoPtr &selectedCard,
                                                                                DeckListModel *deckModel)
{
    if (!selectedCard) {
        return {};
    }

    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();
    QList<QPair<CardInfoPerSet, int>> countList;

    // Collect sets with their counts
    for (const auto &cardInfoPerSetList : cardInfoPerSets) {
        for (const auto &cardInfoPerSet : cardInfoPerSetList) {
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
