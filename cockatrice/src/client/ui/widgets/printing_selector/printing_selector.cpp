#include "printing_selector.h"

#include "../../../../deck/deck_list_model.h"
#include "../../../../game/cards/card_database.h"
#include "../../../../utility/card_set_comparator.h"
#include "printing_selector_card_display_widget.h"

#include <QComboBox>
#include <QHBoxLayout>

PrintingSelector::PrintingSelector(DeckListModel *deckModel, QTreeView *deckView, QWidget *parent)
    : QWidget(parent), deckModel(deckModel), deckView(deckView)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout = new QVBoxLayout();
    setLayout(layout);

    sortOptionsSelector = new QComboBox(this);
    QStringList sortOptions;
    sortOptions << "Alphabetical"
                << "Preference"
                << "Contained in Deck"
                << "Potential Cards in Deck";
    sortOptionsSelector->addItems(sortOptions);
    connect(sortOptionsSelector, SIGNAL(currentTextChanged(QString)), this, SLOT(updateDisplay()));
    layout->addWidget(sortOptionsSelector);

    flowWidget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
    layout->addWidget(flowWidget);
}

void PrintingSelector::updateDisplay()
{
    flowWidget->clearLayout();
    getAllSetsForCurrentCard();
}

void PrintingSelector::setCard(const CardInfoPtr &newCard)
{
    selectedCard = newCard;
    updateDisplay();
}

CardInfoPerSet PrintingSelector::getSetForUUID(const QString &uuid)
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();

    for (const auto &cardInfoPerSet : cardInfoPerSets) {
        if (cardInfoPerSet.getProperty("uuid") == uuid) {
            return cardInfoPerSet;
        }
    }

    return CardInfoPerSet();
}

CardInfoPerSetMap PrintingSelector::sortSets()
{
    CardInfoPerSetMap cardInfoPerSets = selectedCard->getSets();
    if (sortOptionsSelector->currentText() == "Alphabetical") {
        return cardInfoPerSets;
    }

    QList<CardSetPtr> sortedSets;

    for (const auto &set : cardInfoPerSets) {
        sortedSets << set.getPtr();
    }
    if (sortedSets.empty()) {
        sortedSets << CardSet::newInstance("", "", "", QDate());
    }
    std::sort(sortedSets.begin(), sortedSets.end(), SetPriorityComparator());

    auto sortedMap = CardInfoPerSetMap(cardInfoPerSets);
    sortedMap.clear();

    int count = 0;

    for (const auto &set : sortedSets) {
        for (auto it = cardInfoPerSets.begin(); it != cardInfoPerSets.end(); ++it) {
            if (it.value().getPtr() == set) {
                sortedMap.insert(QString::fromStdString(std::to_string(count)), it.value());
                count++;
            }
        }
    }

    return sortedMap;
}

void PrintingSelector::getAllSetsForCurrentCard()
{
    auto sortedMap = sortSets();

    for (auto cardInfoPerSet : sortedMap) {
        auto *cardDisplayWidget =
            new PrintingSelectorCardDisplayWidget(deckModel, deckView, selectedCard, cardInfoPerSet);
        flowWidget->addWidget(cardDisplayWidget);
    }
}
