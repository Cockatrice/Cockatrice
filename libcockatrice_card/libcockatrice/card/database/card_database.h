#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "../set/card_set_list.h"
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
    /// Controller to determine set priority when choosing preferred printings.
    ICardSetPriorityController *setPriorityController;

    /// Cards indexed by exact name
    CardNameMap cards;

    /// Cards indexed by simplified name (normalized)
    CardNameMap simpleNameCards;

    /// Sets indexed by short name
    SetNameMap sets;

    /// Loader responsible for file discovery and parsing
    CardDatabaseLoader *loader;

    /// Current load status of the database
    LoadStatus loadStatus;

    /// Querier for higher-level card lookups
    CardDatabaseQuerier *querier;

private:
    /**
     * @brief Check for sets that are unknown and emit signals if needed.
     */
    void checkUnknownSets();

    /**
     * @brief Refreshes the cached reverse-related cards for all cards.
     */
    void refreshCachedReverseRelatedCards();

    /// Mutexes for thread safety
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
    void removeCard(CardInfoPtr card);

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

    /** @brief Notifies listeners that enabled sets changed. */
    void notifyEnabledSetsChanged();

public slots:
    /**
     * @brief Adds a card to the database.
     * @param card CardInfoPtr to add.
     */
    void addCard(CardInfoPtr card);

    /**
     * @brief Adds a set to the database.
     * @param set Pointer to CardSet to add.
     */
    void addSet(CardSetPtr set);

    /** @brief Loads card databases from configured paths. */
    void loadCardDatabases();

    /** @brief Saves custom tokens to file.
     * @return True if successful.
     */
    bool saveCustomTokensToFile();

signals:
    /** @brief Emitted when the card database has finished loading successfully. */
    void cardDatabaseLoadingFinished();

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
