/**
 * @file card_database_querier.h
 * @ingroup CardDatabase
 * @brief The CardDatabaseQuerier is responsible for querying the database and returning data.
 */

#ifndef COCKATRICE_CARD_DATABASE_QUERIER_H
#define COCKATRICE_CARD_DATABASE_QUERIER_H

#include "../card_info.h"
#include "../printing/exact_card.h"

#include <QObject>
#include <libcockatrice/utility/card_ref.h>

class CardDatabase;
class CardDatabaseQuerier : public QObject
{
    Q_OBJECT

public:
    explicit CardDatabaseQuerier(QObject *parent, const CardDatabase *db);

    [[nodiscard]] CardInfoPtr getCardInfo(const QString &cardName) const;
    [[nodiscard]] QList<CardInfoPtr> getCardInfos(const QStringList &cardNames) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    [[nodiscard]] CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    [[nodiscard]] ExactCard guessCard(const CardRef &cardRef) const;
    [[nodiscard]] ExactCard getCard(const CardRef &cardRef) const;
    [[nodiscard]] QList<ExactCard> getCards(const QList<CardRef> &cardRefs) const;

    [[nodiscard]] ExactCard getRandomCard() const;
    [[nodiscard]] ExactCard getCardFromSameSet(const QString &cardName, const PrintingInfo &otherPrinting) const;

    [[nodiscard]] ExactCard getPreferredCard(const CardInfoPtr &card) const;
    [[nodiscard]] bool isPreferredPrinting(const CardRef &cardRef) const;
    [[nodiscard]] PrintingInfo getPreferredPrinting(const CardInfoPtr &card) const;
    [[nodiscard]] PrintingInfo getPreferredPrinting(const QString &cardName) const;
    [[nodiscard]] QString getPreferredPrintingProviderId(const QString &cardName) const;

    [[nodiscard]] PrintingInfo getSpecificPrinting(const CardRef &cardRef) const;
    [[nodiscard]] PrintingInfo
    getSpecificPrinting(const QString &cardName, const QString &setCode, const QString &collectorNumber) const;
    [[nodiscard]] PrintingInfo findPrintingWithId(const CardInfoPtr &card, const QString &providerId) const;

    [[nodiscard]] QStringList getAllMainCardTypes() const;
    [[nodiscard]] QMap<QString, int> getAllMainCardTypesWithCount() const;
    [[nodiscard]] QMap<QString, int> getAllSubCardTypesWithCount() const;

private:
    const CardDatabase *db;

    CardInfoPtr lookupCardByName(const QString &name) const;
};

#endif // COCKATRICE_CARD_DATABASE_QUERIER_H
