#include "subtype_tally.h"

#include "../board/card_item.h"

#include <QMap>
#include <algorithm>

namespace
{

/** @brief Extracts subtypes from a single card face's type line. */
QStringList extractSubtypesFromFace(const QString &faceType)
{
    // Card type format: "Creature — Goblin Warrior" or "Legendary Enchantment — Saga"
    QStringList parts = faceType.split(QStringLiteral(" — "));
    if (parts.size() > 1) {
        return parts[1].split(QStringLiteral(" "), Qt::SkipEmptyParts);
    }
    return {};
}

/** @brief A single subtype (e.g., "Goblin", "Warrior") with its occurrence count. */
struct SubtypeEntry
{
    QString name;
    int count;
};

} // anonymous namespace

namespace SubtypeTally
{

QList<TallyRow> countSubtypes(const QList<CardItem *> &cards)
{
    QMap<QString, int> subtypeCounts;

    for (CardItem *card : cards) {
        if (card->getFaceDown() || card->getCard().isEmpty()) {
            continue;
        }

        QString cardType = card->getCardInfo().getCardType();
        // Handle double-faced cards: "Creature — Human // Creature — Werewolf"
        QStringList cardFaces = cardType.split(QStringLiteral(" // "));

        for (const QString &face : cardFaces) {
            QStringList subtypes = extractSubtypesFromFace(face);
            for (const QString &subtype : subtypes) {
                subtypeCounts[subtype]++;
            }
        }
    }

    QList<SubtypeEntry> entries;
    for (auto it = subtypeCounts.constBegin(); it != subtypeCounts.constEnd(); ++it) {
        entries.append({it.key(), it.value()});
    }

    // Sort by count ascending, then alphabetically (lowest counts at bottom of display)
    std::sort(entries.begin(), entries.end(), [](const SubtypeEntry &a, const SubtypeEntry &b) {
        if (a.count != b.count) {
            return a.count < b.count;
        }
        return a.name < b.name;
    });

    // convert entries into TallyRows
    QList<TallyRow> rows;
    rows.reserve(entries.size()); // for backwards compatibility with Qt5
    std::transform(entries.begin(), entries.end(), std::back_inserter(rows),
                   [](const SubtypeEntry &e) { return TallyRow{e.name, QString::number(e.count)}; });

    return rows;
}

} // namespace SubtypeTally
