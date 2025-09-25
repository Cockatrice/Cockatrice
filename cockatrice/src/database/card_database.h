#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "../card/exact_card.h"
#include "../common/card_ref.h"
#include "card_database_loader.h"
#include "card_database_queries.h"

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

    CardDatabaseQueries *queries;

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

    const CardNameMap &getCardList() const
    {
        return cards;
    }
    CardSetPtr getSet(const QString &setName);
    SetList getSetList() const;
    LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }
    CardDatabaseQueries *query() const
    {
        return queries;
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
    friend class CardDatabaseQueries;
};

#endif
