#include "stats_tally.h"

#include "../board/card_item.h"

#include <QCoreApplication>
#include <QList>

static int sumPowers(const QList<CardItem *> &cards)
{
    // calculate total power;
    int total = 0;
    for (auto card : cards) {
        QVariantList parsed = CardItem::parsePT(card->getPT());
        if (!parsed.isEmpty()) {
            int power = parsed.first().toInt(); // toInt will default to 0 if it's not an int
            total += qMax(power, 0);
        }
    }
    return total;
}

QList<TallyRow> StatsTally::computeTotalPower(const QList<CardItem *> &cards)
{
    // don't bother if none of the cards have pt
    bool hasPT =
        std::any_of(cards.cbegin(), cards.cend(), [](const CardItem *card) { return !card->getPT().isEmpty(); });
    if (!hasPT) {
        return {};
    }

    int total = sumPowers(cards);

    QString name = QCoreApplication::translate("StatsTally", "Total Power");
    return {TallyRow{name, QString::number(total)}};
}