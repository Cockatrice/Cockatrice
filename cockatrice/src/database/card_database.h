#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "../card/exact_card.h"
#include "../common/card_ref.h"
#include "card_database_loader.h"

#include <QBasicMutex>
#include <QDate>
#include <QHash>
#include <QList>
#include <QLoggingCategory>
#include <QStringList>
#include <QVector>
#include <utility>

inline Q_LOGGING_CATEGORY(CardDatabaseLog, "card_database");

class CardDatabase : public QObject
{
    Q_OBJECT
protected:
    /*
     * The cards, indexed by name.
     */
    CardNameMap cards;

    /**
     * The cards, indexed by their simple name.
     */
    CardNameMap simpleNameCards;

    /*
     * The sets, indexed by short name.
     */
    SetNameMap sets;

    // loader responsible for file discovery & parsing
    CardDatabaseLoader *loader;

    LoadStatus loadStatus;

    QVector<ICardDatabaseParser *> availableParsers;

private:
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();

    QBasicMutex *clearDatabaseMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    explicit CardDatabase(QObject *parent = nullptr);
    ~CardDatabase() override;

    void removeCard(CardInfoPtr card);
    void clear();

    [[nodiscard]] CardInfoPtr getCardInfo(const QString &cardName) const;
    [[nodiscard]] QList<CardInfoPtr> getCardInfos(const QStringList &cardNames) const;

    [[nodiscard]] ExactCard getCard(const CardRef &cardRef) const;
    [[nodiscard]] QList<ExactCard> getCards(const QList<CardRef> &cardRefs) const;

    [[nodiscard]] ExactCard getPreferredCard(const CardInfoPtr &cardInfo) const;
    [[nodiscard]] PrintingInfo getPreferredPrinting(const QString &cardName) const;
    [[nodiscard]] PrintingInfo getPreferredPrinting(const CardInfoPtr &cardInfo) const;
    QString getPreferredPrintingProviderId(const QString &cardName) const;
    bool isPreferredPrinting(const CardRef &cardRef) const;

    static PrintingInfo findPrintingWithId(const CardInfoPtr &cardInfo, const QString &providerId);
    [[nodiscard]] PrintingInfo getSpecificPrinting(const CardRef &cardRef) const;
    PrintingInfo
    getSpecificPrinting(const QString &cardName, const QString &setShortName, const QString &collectorNumber) const;

    ExactCard getCardFromSameSet(const QString &cardName, const PrintingInfo &otherPrinting) const;

    [[nodiscard]] ExactCard guessCard(const CardRef &cardRef) const;
    [[nodiscard]] ExactCard getRandomCard();

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    [[nodiscard]] CardInfoPtr getCardBySimpleName(const QString &cardName) const;
    CardInfoPtr lookupCardByName(const QString &name) const;

    CardSetPtr getSet(const QString &setName);
    const CardNameMap &getCardList() const
    {
        return cards;
    }
    SetList getSetList() const;
    CardInfoPtr getCardFromMap(const CardNameMap &cardMap, const QString &cardName) const;
    QStringList getAllMainCardTypes() const;
    QMap<QString, int> getAllMainCardTypesWithCount() const;
    QMap<QString, int> getAllSubCardTypesWithCount() const;
    LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }
    void enableAllUnknownSets();
    void markAllSetsAsKnown();
    void notifyEnabledSetsChanged();

public slots:
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
    void loadCardDatabases();
    bool saveCustomTokensToFile();
signals:
    void cardDatabaseLoadingFinished();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();
    void cardDatabaseEnabledSetsChanged();
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);

    friend class CardDatabaseLoader;
};

#endif
