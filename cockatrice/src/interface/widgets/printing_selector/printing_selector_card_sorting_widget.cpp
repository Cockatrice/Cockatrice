#include "printing_selector_card_sorting_widget.h"

#include <libcockatrice/card/set/card_set_comparator.h>
#include <libcockatrice/settings/cache_settings.h>

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
    setMinimumWidth(300);
    sortToolBar = new QHBoxLayout(this);

    sortOptionsSelector = new QComboBox(this);
    sortOptionsSelector->setFocusPolicy(Qt::StrongFocus);
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
 * @param setMap The list of card sets to be sorted.
 * @return A sorted list of printings.
 */
QList<PrintingInfo> PrintingSelectorCardSortingWidget::sortSets(const SetToPrintingsMap &setMap)
{
    QList<CardSetPtr> sortedSets;

    for (const auto &printingInfos : setMap) {
        for (const auto &set : printingInfos) {
            sortedSets << set.getSet();
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

    QList<PrintingInfo> sortedPrintings;
    // Reconstruct sorted list of PrintingInfo
    for (const auto &set : sortedSets) {
        for (auto it = setMap.begin(); it != setMap.end(); ++it) {
            for (const auto &printingInfo : it.value()) {
                if (printingInfo.getSet() == set) {
                    if (!sortedPrintings.contains(printingInfo)) {
                        sortedPrintings << printingInfo;
                    }
                }
            }
        }
    }

    if (descendingSort) {
        std::reverse(sortedPrintings.begin(), sortedPrintings.end());
    }

    return sortedPrintings;
}

/**
 * @brief Filters a list of card sets based on the search text.
 *
 * This function filters the given list of card sets by comparing their long and short names with the provided search
 * text. If the search text matches either the long or short name of a card set, that set is included in the filtered
 * list.
 *
 * @param printings The list of printings to be filtered.
 * @param searchText The search text used to filter the card sets.
 * @return A filtered list of card sets.
 */
QList<PrintingInfo> PrintingSelectorCardSortingWidget::filterSets(const QList<PrintingInfo> &printings,
                                                                  const QString &searchText)
{
    if (searchText.isEmpty()) {
        return printings;
    }

    QList<PrintingInfo> filteredPrintings;

    for (const auto &printing : printings) {
        const QString longName = printing.getSet()->getLongName().toLower();
        const QString shortName = printing.getSet()->getShortName().toLower();

        if (longName.contains(searchText) || shortName.contains(searchText)) {
            filteredPrintings << printing;
        }
    }

    return filteredPrintings;
}

QList<PrintingInfo> PrintingSelectorCardSortingWidget::prependPinnedPrintings(const QList<PrintingInfo> &printings,
                                                                              const QString &cardName)
{
    auto printingsToUse = printings;
    const auto &cardProviderId = SettingsCache::instance().cardOverrides().getCardPreferenceOverride(cardName);
    if (!cardProviderId.isEmpty()) {
        for (int i = 0; i < printingsToUse.size(); ++i) {
            const auto &card = printingsToUse[i];
            if (card.getUuid() == cardProviderId) {
                printingsToUse.move(i, 0);
                break;
            }
        }
    }

    return printingsToUse;
}

/**
 * @brief Prepend card printings that are contained in the deck to the list of printings.
 *
 * This function adjusts the list of printings by moving the printings that are already contained in the deck to the
 * beginning of the list, sorted by the count of cards in the deck.
 *
 * @param printings The original list of printings.
 * @param selectedCard The currently selected card.
 * @param deckModel The model representing the deck.
 * @return A list of printings with the printings contained in the deck prepended.
 */
QList<PrintingInfo> PrintingSelectorCardSortingWidget::prependPrintingsInDeck(const QList<PrintingInfo> &printings,
                                                                              const CardInfoPtr &selectedCard,
                                                                              DeckListModel *deckModel)
{
    if (!selectedCard) {
        return {};
    }

    SetToPrintingsMap setMap = selectedCard->getSets();
    QList<QPair<PrintingInfo, int>> countList;

    // Collect sets with their counts
    for (const auto &printingList : setMap) {
        for (const auto &printing : printingList) {
            QModelIndex find_card =
                deckModel->findCard(selectedCard->getName(), DECK_ZONE_MAIN, printing.getProperty("uuid"));
            if (find_card.isValid()) {
                int count =
                    deckModel->data(find_card, Qt::DisplayRole).toInt(); // Ensure the count is treated as an integer
                if (count > 0) {
                    countList.append(qMakePair(printing, count));
                }
            }
            break;
        }
    }

    // Sort sets by count in descending numerical order
    std::sort(countList.begin(), countList.end(),
              [](const QPair<PrintingInfo, int> &a, const QPair<PrintingInfo, int> &b) {
                  return a.second > b.second; // Ensure numerical comparison
              });

    // Create a copy of the original list to modify
    QList<PrintingInfo> result = printings;

    // Prepend sorted sets and remove them from the original list
    for (const auto &pair : countList) {
        auto it = std::find_if(result.begin(), result.end(),
                               [&pair](const PrintingInfo &item) { return item.getUuid() == pair.first.getUuid(); });
        if (it != result.end()) {
            result.erase(it); // Remove the matching entry
        }
        result.prepend(pair.first); // Prepend the sorted item
    }

    return result;
}
