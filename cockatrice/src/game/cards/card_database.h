#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "card_info.h"

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

typedef QHash<QString, CardInfoPtr> CardNameMap;
typedef QHash<QString, CardSetPtr> SetNameMap;

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
    CardInfoPtr getCardFromMap(const CardNameMap &cardMap, const QString &cardName) const;
    void checkUnknownSets();
    void refreshCachedReverseRelatedCards();

    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(), *clearDatabaseMutex = new QBasicMutex(),
                *loadFromFileMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    static const char *TOKENS_SETNAME;

    explicit CardDatabase(QObject *parent = nullptr);
    ~CardDatabase() override;
    void clear();
    void removeCard(CardInfoPtr card);
    [[nodiscard]] CardInfoPtr getCard(const QString &cardName) const;
    [[nodiscard]] QList<CardInfoPtr> getCards(const QStringList &cardNames) const;
    QList<CardInfoPtr> getCardsByNameAndProviderId(const QMap<QString, QString> &cardNames) const;
    [[nodiscard]] CardInfoPtr getCardByNameAndProviderId(const QString &cardName, const QString &providerId) const;
    [[nodiscard]] CardInfoPerSet getPreferredSetForCard(const QString &cardName) const;
    [[nodiscard]] CardInfoPerSet getSpecificSetForCard(const QString &cardName, const QString &providerId) const;
    CardInfoPerSet
    getSpecificSetForCard(const QString &cardName, const QString &setShortName, const QString &collectorNumber) const;
    QString getPreferredPrintingProviderIdForCard(const QString &cardName);
    [[nodiscard]] CardInfoPtr guessCard(const QString &cardName, const QString &providerId = QString()) const;

    /*
     * Get a card by its simple name. The name will be simplified in this
     * function, so you don't need to simplify it beforehand.
     */
    [[nodiscard]] CardInfoPtr getCardBySimpleName(const QString &cardName) const;

    CardSetPtr getSet(const QString &setName);
    bool isProviderIdForPreferredPrinting(const QString &cardName, const QString &providerId);
    static CardInfoPerSet getSetInfoForCard(const CardInfoPtr &_card);
    const CardNameMap &getCardList() const
    {
        return cards;
    }
    SetList getSetList() const;
    LoadStatus loadFromFile(const QString &fileName);
    bool saveCustomTokensToFile();
    QStringList getAllMainCardTypes() const;
    QMap<QString, int> getAllMainCardTypesWithCount() const;
    LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }
    void enableAllUnknownSets();
    void markAllSetsAsKnown();
    void notifyEnabledSetsChanged();

public slots:
    LoadStatus loadCardDatabases();
    void refreshPreferredPrintings();
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
