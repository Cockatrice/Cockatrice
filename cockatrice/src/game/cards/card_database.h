#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "../common/card_ref.h"
#include "exact_card.h"

#include <QBasicMutex>
#include <QDate>
#include <QHash>
#include <QList>
#include <QLoggingCategory>
#include <QStringList>
#include <QVector>
#include <utility>

inline Q_LOGGING_CATEGORY(CardDatabaseLog, "card_database");
inline Q_LOGGING_CATEGORY(CardDatabaseLoadingLog, "card_database.loading");
inline Q_LOGGING_CATEGORY(CardDatabaseLoadingSuccessOrFailureLog, "card_database.loading.success_or_failure");

class ICardDatabaseParser;

enum LoadStatus
{
    Ok,
    VersionTooOld,
    Invalid,
    NotLoaded,
    FileError,
    NoCards
};

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

    LoadStatus loadStatus;

    QVector<ICardDatabaseParser *> availableParsers;

private:
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();

    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(), *clearDatabaseMutex = new QBasicMutex(),
                *loadFromFileMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    explicit CardDatabase(QObject *parent = nullptr);
    ~CardDatabase() override;
    void clear();
    void removeCard(CardInfoPtr card);

    [[nodiscard]] CardInfoPtr getCardInfo(const QString &cardName) const;
    [[nodiscard]] QList<CardInfoPtr> getCardInfos(const QStringList &cardNames) const;

    QList<ExactCard> getCards(const QList<CardRef> &cardRefs) const;
    [[nodiscard]] ExactCard getCard(const CardRef &cardRef) const;

    [[nodiscard]] ExactCard getPreferredCard(const CardInfoPtr &cardInfo) const;

    static PrintingInfo findPrintingWithId(const CardInfoPtr &cardInfo, const QString &providerId);
    [[nodiscard]] PrintingInfo getPreferredPrinting(const QString &cardName) const;
    [[nodiscard]] PrintingInfo getPreferredPrinting(const CardInfoPtr &cardInfo) const;
    [[nodiscard]] PrintingInfo getSpecificPrinting(const CardRef &cardRef) const;
    PrintingInfo
    getSpecificPrinting(const QString &cardName, const QString &setShortName, const QString &collectorNumber) const;
    QString getPreferredPrintingProviderId(const QString &cardName) const;
    bool isPreferredPrinting(const CardRef &cardRef) const;

    [[nodiscard]] ExactCard guessCard(const CardRef &cardRef) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    [[nodiscard]] CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    CardSetPtr getSet(const QString &setName);
    const CardNameMap &getCardList() const
    {
        return cards;
    }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName);
    bool saveCustomTokensToFile();
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
    LoadStatus loadCardDatabases();
    void addCard(CardInfoPtr card);
    void addSet(CardSetPtr set);
protected slots:
    LoadStatus loadCardDatabase(const QString &path);
signals:
    void cardDatabaseLoadingFinished();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();
    void cardDatabaseEnabledSetsChanged();
    void cardAdded(CardInfoPtr card);
    void cardRemoved(CardInfoPtr card);
};

#endif
