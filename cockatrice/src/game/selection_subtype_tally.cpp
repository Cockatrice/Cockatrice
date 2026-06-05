#include "selection_subtype_tally.h"

#include "board/card_item.h"

#include <QMap>
#include <algorithm>

namespace
{

QStringList extractSubtypesFromFace(const QString &faceType)
{
    QStringList parts = faceType.split(QStringLiteral(" — "));
    if (parts.size() > 1) {
        return parts[1].split(QStringLiteral(" "), Qt::SkipEmptyParts);
    }
    return {};
}

} // anonymous namespace

namespace SelectionSubtypeTally
{

QList<MainTypeGroup> countSubtypes(const QList<CardItem *> &cards)
{
    QMap<QString, QMap<QString, int>> subtypesByMainType;
    QMap<QString, int> cardCountPerMainType;

    for (CardItem *card : cards) {
        if (card->getFaceDown() || card->getCard().isEmpty()) {
            continue;
        }

        QString mainType = card->getCardInfo().getMainCardType();
        if (mainType.isEmpty()) {
            mainType = QStringLiteral("Other");
        }

        QString cardType = card->getCardInfo().getCardType();
        QStringList cardFaces = cardType.split(QStringLiteral(" // "));

        bool contributedSubtypes = false;
        for (const QString &face : cardFaces) {
            QStringList subtypes = extractSubtypesFromFace(face);
            for (const QString &subtype : subtypes) {
                subtypesByMainType[mainType][subtype]++;
                contributedSubtypes = true;
            }
        }

        if (contributedSubtypes) {
            cardCountPerMainType[mainType]++;
        }
    }

    QList<MainTypeGroup> groups;
    for (auto it = subtypesByMainType.constBegin(); it != subtypesByMainType.constEnd(); ++it) {
        MainTypeGroup group;
        group.mainType = it.key();
        group.cardCount = cardCountPerMainType.value(it.key(), 0);

        for (auto subIt = it.value().constBegin(); subIt != it.value().constEnd(); ++subIt) {
            group.subtypes.append({subIt.key(), subIt.value()});
        }

        // Sort subtypes: by count ascending, then alphabetically
        std::sort(group.subtypes.begin(), group.subtypes.end(), [](const SubtypeEntry &a, const SubtypeEntry &b) {
            if (a.count != b.count) {
                return a.count < b.count;
            }
            return a.name < b.name;
        });

        groups.append(group);
    }

    // Sort groups: by card count ascending, then alphabetically by main type
    std::sort(groups.begin(), groups.end(), [](const MainTypeGroup &a, const MainTypeGroup &b) {
        if (a.cardCount != b.cardCount) {
            return a.cardCount < b.cardCount;
        }
        return a.mainType < b.mainType;
    });

    return groups;
}

} // namespace SelectionSubtypeTally
