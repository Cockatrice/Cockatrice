#include "tally_manager.h"

#include "../board/card_item.h"

const QList<TallyManager::EntryType> TallyManager::ENTRY_TYPES = {COUNT, TOTAL_POWER, TOTAL_MANA_VALUE};

TallyManager::TallyManager(QObject *parent) : QObject(parent)
{
    for (auto entryType : ENTRY_TYPES) {
        resultsMap.insert(entryType, {});
    }
    updateResultsMap({});
}

const QMap<TallyManager::EntryType, TallyResult> &TallyManager::getAllResults() const
{
    return resultsMap;
}

void TallyManager::updateSelection(const QList<CardItem *> &cards)
{
    updateResultsMap(cards);
    emit tallyChanged(resultsMap);
}

void TallyManager::updateResultsMap(const QList<CardItem *> &cards)
{
    for (auto i = resultsMap.begin(); i != resultsMap.end(); ++i) {
        i.value() = computeTallyResult(i.key(), cards);
    }
}

static int totalCount(const QList<CardItem *> &cards)
{
    return cards.count();
}

static int totalPower(const QList<CardItem *> &cards)
{
    static const auto powerRegex = QRegularExpression("(\\d+)/(\\d+)");
    int total = 0;
    for (auto card : cards) {
        QString pt = card->getPT();
        auto match = powerRegex.match(pt);
        if (match.hasMatch()) {
            int power = match.captured(1).toInt(); // defaults to 0 if string can't be parsed as int
            total += power;
        }
    }
    return total;
}

static int totalManaValue(const QList<CardItem *> &cards)
{
    int total = 0;
    for (auto card : cards) {
        int mv = card->getCardInfo().getCmc().toInt();
        total += mv;
    }
    return total;
}

TallyResult TallyManager::computeTallyResult(EntryType type, const QList<CardItem *> &cards)
{
    switch (type) {
        case COUNT:
            return {tr("Count"), totalCount(cards)};
        case TOTAL_POWER:
            return {tr("Total Power"), totalPower(cards)};
        case TOTAL_MANA_VALUE:
            return {tr("Total Mana Value"), totalManaValue(cards)};
    }
    return {};
}
