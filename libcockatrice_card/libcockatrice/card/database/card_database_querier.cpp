#include "card_database_querier.h"

#include "../card_info.h"
#include "../printing/exact_card.h"
#include "../set/card_set_comparator.h"
#include "card_database.h"

#include <qrandom.h>

CardDatabaseQuerier::CardDatabaseQuerier(QObject *_parent,
                                         const CardDatabase *_db,
                                         const ICardPreferenceProvider *prefs)
    : QObject(_parent), db(_db), prefs(prefs)
{
}

/**
 * Looks up the cardInfo corresponding to the cardName.
 *
 * @param cardName The card name to look up
 * @return A CardInfoPtr, or null if not corresponding CardInfo is found.
 */
CardInfoPtr CardDatabaseQuerier::getCardInfo(const QString &cardName) const
{
    return db->cards.value(cardName);
}

/**
 * Looks up the cardInfos for a list of card names.
 *
 * @param cardNames The card names to look up
 * @return A List of CardInfoPtr. Any failed lookups will be ignored and dropped from the resulting list
 */
QList<CardInfoPtr> CardDatabaseQuerier::getCardInfos(const QStringList &cardNames) const
{
    QList<CardInfoPtr> cardInfos;
    for (const QString &cardName : cardNames) {
        CardInfoPtr ptr = db->cards.value(cardName);
        if (ptr)
            cardInfos.append(ptr);
    }

    return cardInfos;
}

CardInfoPtr CardDatabaseQuerier::getCardBySimpleName(const QString &cardName) const
{
    return db->simpleNameCards.value(CardInfo::simplifyName(cardName));
}

CardInfoPtr CardDatabaseQuerier::lookupCardByName(const QString &name) const
{
    if (auto info = getCardInfo(name))
        return info;
    if (auto info = getCardBySimpleName(name))
        return info;
    return getCardBySimpleName(CardInfo::simplifyName(name));
}

/**
 * Looks up the cards corresponding to the CardRefs.
 * If the providerId is empty, will default to the preferred printing.
 * If providerId is given but not found, the PrintingInfo will be empty.
 *
 * @param cardRefs The cards to look up. If providerId is empty for an entry, will default to the preferred printing for
 * that entry. If providerId is given but not found, the PrintingInfo will be empty for that entry.
 * @return A list of cards. Any failed lookups will be ignored and dropped from the resulting list.
 */
QList<ExactCard> CardDatabaseQuerier::getCards(const QList<CardRef> &cardRefs) const
{
    QList<ExactCard> cards;
    for (const auto &cardRef : cardRefs) {
        ExactCard card = getCard(cardRef);
        if (card)
            cards.append(card);
    }

    return cards;
}

/**
 * Looks up the card corresponding to the CardRef.
 * If the providerId is empty, will default to the preferred printing.
 * If providerId is given but not found, the PrintingInfo will be empty.
 *
 * @param cardRef The card to look up.
 * @return A specific printing of a card, or empty if not found.
 */
ExactCard CardDatabaseQuerier::getCard(const CardRef &cardRef) const
{
    auto info = getCardInfo(cardRef.name);
    if (info.isNull()) {
        return {};
    }

    if (cardRef.providerId.isEmpty() || cardRef.providerId.isNull()) {
        return ExactCard(info, getPreferredPrinting(info));
    }

    return ExactCard(info, findPrintingWithId(info, cardRef.providerId));
}

/**
 * Looks up the card by CardRef, simplifying the name if required.
 * If the providerId is empty, will default to the preferred printing.
 * If providerId is given but not found, the PrintingInfo will be empty.
 *
 * @param cardRef The card to look up.
 * @return A specific printing of a card, or empty if not found.
 */
ExactCard CardDatabaseQuerier::guessCard(const CardRef &cardRef) const
{
    auto card = lookupCardByName(cardRef.name);
    auto printing =
        cardRef.providerId.isEmpty() ? getPreferredPrinting(card) : findPrintingWithId(card, cardRef.providerId);

    return ExactCard(card, printing);
}

ExactCard CardDatabaseQuerier::getRandomCard() const
{
    if (db->cards.isEmpty())
        return {};

    const auto keys = db->cards.keys();
    int randomIndex = QRandomGenerator::global()->bounded(keys.size());
    const QString &randomKey = keys.at(randomIndex);
    CardInfoPtr randomCard = getCardInfo(randomKey);

    return ExactCard{randomCard, getPreferredPrinting(randomCard)};
}

ExactCard CardDatabaseQuerier::getCardFromSameSet(const QString &cardName, const PrintingInfo &otherPrinting) const
{
    // The source card does not have a printing defined, which means we can't get a card from the same set.
    if (otherPrinting == PrintingInfo()) {
        return getCard({cardName});
    }

    // The source card does have a printing defined, which means we can attempt to get a card from the same set.
    PrintingInfo relatedPrinting = getSpecificPrinting(cardName, otherPrinting.getSet()->getCorrectedShortName(), "");
    ExactCard relatedCard(guessCard({cardName}).getCardPtr(), relatedPrinting);

    // If we didn't find a card from the same set, just try to find any card with the same name.
    return relatedCard ? relatedCard : getCard({cardName});
}

/**
 * Finds the PrintingInfo in the cardInfo that has the given uuid field.
 *
 * @param cardInfo The CardInfo to search
 * @param providerId The uuid to look for
 * @return The PrintingInfo, or a default-constructed PrintingInfo if not found.
 */
PrintingInfo CardDatabaseQuerier::findPrintingWithId(const CardInfoPtr &cardInfo, const QString &providerId) const
{
    for (const auto &printings : cardInfo->getSets()) {
        for (const auto &printing : printings) {
            if (printing.getUuid() == providerId) {
                return printing;
            }
        }
    }

    return PrintingInfo();
}

PrintingInfo CardDatabaseQuerier::getSpecificPrinting(const CardRef &cardRef) const
{
    CardInfoPtr cardInfo = getCardInfo(cardRef.name);
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    return findPrintingWithId(cardInfo, cardRef.providerId);
}

PrintingInfo CardDatabaseQuerier::getSpecificPrinting(const QString &cardName,
                                                      const QString &setShortName,
                                                      const QString &collectorNumber) const
{
    CardInfoPtr cardInfo = getCardInfo(cardName);
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    SetToPrintingsMap setMap = cardInfo->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    for (const auto &printings : setMap) {
        for (auto &cardInfoForSet : printings) {
            if (!collectorNumber.isEmpty()) {
                if (cardInfoForSet.getSet()->getShortName() == setShortName &&
                    cardInfoForSet.getProperty("num") == collectorNumber) {
                    return cardInfoForSet;
                }
            } else {
                if (cardInfoForSet.getSet()->getShortName() == setShortName) {
                    return cardInfoForSet;
                }
            }
        }
    }

    return PrintingInfo(nullptr);
}

/**
 * Gets the card representing the preferred printing of the cardInfo
 *
 * @param cardName The cardName to find the preferred card and printing for
 * @return A specific printing of a card
 */
ExactCard CardDatabaseQuerier::getPreferredCard(const QString &cardName) const
{
    return getPreferredCard(getCardInfo(cardName));
}

/**
 * Gets the card representing the preferred printing of the cardInfo
 *
 * @param cardInfo The cardInfo to find the preferred printing for
 * @return A specific printing of a card
 */
ExactCard CardDatabaseQuerier::getPreferredCard(const CardInfoPtr &cardInfo) const
{
    return ExactCard(cardInfo, getPreferredPrinting(cardInfo));
}

bool CardDatabaseQuerier::isPreferredPrinting(const CardRef &cardRef) const
{
    if (cardRef.providerId.startsWith("card_")) {
        return cardRef.providerId ==
               QLatin1String("card_") + cardRef.name + QString("_") + getPreferredPrintingProviderId(cardRef.name);
    }
    return cardRef.providerId == getPreferredPrintingProviderId(cardRef.name);
}

PrintingInfo CardDatabaseQuerier::getPreferredPrinting(const QString &cardName) const
{
    CardInfoPtr cardInfo = getCardInfo(cardName);
    return getPreferredPrinting(cardInfo);
}

PrintingInfo CardDatabaseQuerier::getPreferredPrinting(const CardInfoPtr &cardInfo) const
{
    if (!cardInfo) {
        return PrintingInfo(nullptr);
    }

    const auto &pinnedPrintingProviderId = prefs->getCardPreferenceOverride(cardInfo->getName());

    if (!pinnedPrintingProviderId.isEmpty()) {
        return getSpecificPrinting({cardInfo->getName(), pinnedPrintingProviderId});
    }

    SetToPrintingsMap setMap = cardInfo->getSets();
    if (setMap.empty()) {
        return PrintingInfo(nullptr);
    }

    CardSetPtr preferredSet = nullptr;
    PrintingInfo preferredPrinting;
    SetPriorityComparator comparator;

    for (const auto &printings : setMap) {
        for (auto &printing : printings) {
            CardSetPtr currentSet = printing.getSet();
            if (!preferredSet || comparator(currentSet, preferredSet)) {
                preferredSet = currentSet;
                preferredPrinting = printing;
            }
        }
    }

    if (preferredSet) {
        return preferredPrinting;
    }

    return PrintingInfo(nullptr);
}

QString CardDatabaseQuerier::getPreferredPrintingProviderId(const QString &cardName) const
{
    PrintingInfo preferredPrinting = getPreferredPrinting(cardName);
    QString uuid = preferredPrinting.getUuid();
    if (!uuid.isEmpty()) {
        return uuid;
    }

    CardInfoPtr defaultCardInfo = getCardInfo(cardName);
    if (defaultCardInfo.isNull()) {
        return cardName;
    }
    return defaultCardInfo->getName();
}

QStringList CardDatabaseQuerier::getAllMainCardTypes() const
{
    QSet<QString> types;
    for (const auto &card : db->cards.values()) {
        types.insert(card->getMainCardType());
    }
    return types.values();
}

QMap<QString, int> CardDatabaseQuerier::getAllMainCardTypesWithCount() const
{
    QMap<QString, int> typeCounts;

    for (const auto &card : db->cards.values()) {
        QString type = card->getMainCardType();
        typeCounts[type]++;
    }

    return typeCounts;
}

QMap<QString, int> CardDatabaseQuerier::getAllSubCardTypesWithCount() const
{
    QMap<QString, int> typeCounts;

    for (const auto &card : db->cards.values()) {
        QString type = card->getCardType();

        QStringList parts = type.split(" — ");

        if (parts.size() > 1) { // Ensure there are subtypes
            QStringList subtypes = parts[1].split(" ", Qt::SkipEmptyParts);

            for (const QString &subtype : subtypes) {
                typeCounts[subtype]++;
            }
        }
    }

    return typeCounts;
}

FormatRulesPtr CardDatabaseQuerier::getFormat(const QString &formatName) const
{
    return db->formats.value(formatName.toLower());
}

QMap<QString, int> CardDatabaseQuerier::getAllFormatsWithCount() const
{
    QMap<QString, int> formatCounts;

    for (const auto &card : db->cards.values()) {
        QStringList allProps = card->getProperties();

        for (const QString &prop : allProps) {
            if (prop.startsWith("format-")) {
                QString formatName = prop.mid(QStringLiteral("format-").size());
                formatCounts[formatName]++;
            }
        }
    }

    return formatCounts;
}