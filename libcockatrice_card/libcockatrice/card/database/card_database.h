#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "../set/card_set_list.h"
#include "card_database_data.h"
#include "card_database_loader.h"
#include "card_database_querier.h"

#include <QBasicMutex>
#include <QDate>
#include <QHash>
#include <QList>
#include <QLoggingCategory>
#include <QVector>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <utility>

inline Q_LOGGING_CATEGORY(CardDatabaseLog, "card_database");

/**
 * @class CardDatabase
 * @ingroup CardDatabase
 * @brief Core in-memory container for card and set data.
 *
 * Responsible for maintaining CardInfo objects, CardSet objects, and
 * providing access to CardDatabaseQuerier for query operations.
 * Handles addition, removal, and clearing of cards and sets.
 */
class CardDatabase : public QObject
{
    Q_OBJECT

protected:
    /** @brief Controller to determine set priority when choosing preferred printings. */
    ICardSetPriorityController *setPriorityController;

    /** @brief Cards indexed by exact name. */
    CardNameMap cards;

    /** @brief Cards indexed by simplified name (normalized). */
    CardNameMap simpleNameCards;

    /** @brief Sets indexed by short name. */
    SetNameMap sets;

    FormatRulesNameMap formats;

    /** @brief Loader responsible for file discovery and parsing. */
    CardDatabaseLoader *loader;

    /** @brief Current load status of the database. */
    LoadStatus loadStatus;

    /** @brief Querier for higher-level card lookups. */
    CardDatabaseQuerier *querier;

private:
    /**
     * @brief Refreshes the cached reverse-related cards for all cards.
     */
    void refreshCachedReverseRelatedCards();

    /**
     * @brief Refreshes the cached reverse-related cards for the given card map.
     * @param cardMap Map of cards to process.
     */
    void refreshCachedReverseRelatedCards(CardNameMap &cardMap);

    /** @brief Mutexes for thread safety. */
    QBasicMutex *clearDatabaseMutex = new QBasicMutex(), *addCardMutex = new QBasicMutex(),
                *removeCardMutex = new QBasicMutex();

public:
    /**
     * @brief Constructs a new CardDatabase instance.
     * @param parent QObject parent.
     * @param prefs Optional card preference provider.
     * @param pathProvider Optional database path provider.
     * @param setPriorityController Optional controller for set priority.
     */
    explicit CardDatabase(QObject *parent = nullptr,
                          ICardPreferenceProvider *prefs = nullptr,
                          ICardDatabasePathProvider *pathProvider = nullptr,
                          ICardSetPriorityController *setPriorityController = nullptr);

    /** @brief Destructor clears all internal data. */
    ~CardDatabase() override;

    /**
     * @brief Removes a card from the database.
     * @param card Pointer to the card to remove.
     */
    void removeCard(const CardInfoPtr &card);

    /** @brief Clears all cards, sets, and internal state. */
    void clear();

    /** @brief Returns the map of cards by name. */
    [[nodiscard]] const CardNameMap &getCardList() const
    {
        return cards;
    }

    /**
     * @brief Retrieves a set by short name, creating a new one if missing.
     * @param setName Short name of the set.
     * @return Pointer to the CardSet.
     */
    CardSetPtr getSet(const QString &setName);

    /** @brief Returns a list of all sets in the database. */
    [[nodiscard]] CardSetList getSetList() const;

    /** @brief Returns the current load status. */
    [[nodiscard]] LoadStatus getLoadStatus() const
    {
        return loadStatus;
    }

    /** @brief Returns the querier for performing card lookups. */
    [[nodiscard]] CardDatabaseQuerier *query() const
    {
        return querier;
    }

    /** @brief Enables all unknown sets in the database. */
    void enableAllUnknownSets();

    /** @brief Marks all sets as known. */
    void markAllSetsAsKnown();

    /**
     * @brief Notifies listeners that enabled sets changed.
     * @param recomputeCachedSets When true, every card's cached set-name / alt-name
     *        data is recomputed (needed only when enabled-set state actually
     *        changed). Pass false after a plain reload, where enabled-set state is
     *        unchanged and the cached data is still valid.
     */
    void notifyEnabledSetsChanged(bool recomputeCachedSets = true);

    /**
     * @brief Check for sets that are unknown and emit signals if needed.
     *
     * Called from MainWindow::startupConfigCheck() after signal connections are
     * live (the front-loaded parse in main() runs before MainWindow exists, so
     * signals emitted there would have no receivers).  May also be called by
     * the loader via the friend declaration.  Has side effects: enables all
     * sets when none are enabled (first-run), marks all sets as known.
     */
    void checkUnknownSets();

    ICardSetPriorityController *getPriorityController()
    {
        return setPriorityController;
    }

public slots:
    /**
     * @brief Adds a card to the database.
     * @param card CardInfoPtr to add.
     */
    void addCard(const CardInfoPtr &card);

    /**
     * @brief Adds a set to the database.
     * @param set Pointer to CardSet to add.
     */
    void addSet(const CardSetPtr &set);

    void addFormat(const FormatRulesPtr &format);

    /** @brief Loads card databases from configured paths. */
    void loadCardDatabases();
    void reloadCardDatabasesAndNotify();

    /** @brief Saves custom tokens to file.
     * @return True if successful.
     */
    bool saveCustomTokensToFile();

    /**
     * @brief Replaces the entire contents of the database with the provided
     *        snapshot in a single operation. Intended to be called once a
     *        background load has finished building a CardDatabaseData, so that
     *        observers never see a partially-populated database.
     * @param data The fully-built snapshot to adopt.
     */
    void swapInDatabaseData(CardDatabaseData data);

signals:
    /** @brief Emitted when the card database has finished loading successfully. */
    void cardDatabaseLoadingFinished();

    /**
     * @brief Emitted once after the live database contents have been replaced
     *        by a freshly loaded snapshot. Models should rebuild from the
     *        database in a single batch rather than reacting to per-card adds.
     */
    void cardDatabaseReset();

    /** @brief Emitted when the card database fails to load. */
    void cardDatabaseLoadingFailed();

    /**
     * @brief Emitted when new sets are found.
     * @param numUnknownSets Number of unknown sets.
     * @param unknownSetsNames Names of unknown sets.
     */
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);

    /** @brief Emitted when all new sets have been enabled. */
    void cardDatabaseAllNewSetsEnabled();

    /** @brief Emitted when enabled sets have changed. */
    void cardDatabaseEnabledSetsChanged();

    /** @brief Emitted when a new card is added. */
    void cardAdded(CardInfoPtr card);

    /** @brief Emitted when a card is removed. */
    void cardRemoved(CardInfoPtr card);

    friend class CardDatabaseLoader;
    friend class CardDatabaseQuerier;
};

#endif // CARDDATABASE_H
