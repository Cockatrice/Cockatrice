#ifndef COCKATRICE_CARD_DATABASE_LOADER_H
#define COCKATRICE_CARD_DATABASE_LOADER_H

#include <QBasicMutex>
#include <QList>
#include <QLoggingCategory>
#include <libcockatrice/card/database/card_database_data.h>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_card_preference_provider.h>
#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>

inline Q_LOGGING_CATEGORY(CardDatabaseLoadingLog, "card_database.loading");
inline Q_LOGGING_CATEGORY(CardDatabaseLoadingSuccessOrFailureLog, "card_database.loading.success_or_failure");

class CardDatabase;
class ICardDatabaseParser;

/**
 * @enum LoadStatus
 * @brief Represents the result of attempting to load a card database.
 */
enum LoadStatus
{
    Ok,            /**< Database loaded successfully. */
    VersionTooOld, /**< Database version is too old to load. */
    Invalid,       /**< Database is invalid or unparsable. */
    NotLoaded,     /**< Database has not been loaded. */
    FileError,     /**< Error opening or reading the file. */
    NoCards        /**< Database contains no cards. */
};

/**
 * @class CardDatabaseLoader
 * @ingroup CardDatabase
 * @brief Handles loading card databases from disk and saving custom tokens.
 *
 * This class is responsible for:
 *  - Discovering configured card database paths.
 *  - Loading main, token, spoiler, and custom databases.
 *  - Populating a CardDatabase instance using connected parsers.
 *  - Emitting signals about loading progress and new sets.
 */
class CardDatabaseLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a CardDatabaseLoader.
     * @param parent QObject parent.
     * @param db Pointer to the CardDatabase to populate (non-owning).
     * @param pathProvider Provider for card database file paths.
     * @param preferenceProvider Optional card preference provider for pinned printings.
     */
    explicit CardDatabaseLoader(QObject *parent,
                                CardDatabase *db,
                                ICardDatabasePathProvider *pathProvider,
                                ICardPreferenceProvider *preferenceProvider,
                                ICardSetPriorityController *_priorityController);

    /** @brief Destructor cleans up allocated parsers. */
    ~CardDatabaseLoader() override;

public slots:
    /**
     * @brief Loads all configured card databases.
     *
     * Runs synchronously on the calling thread.  The caller should ensure
     * that any signal receivers are already connected before invoking this.
     * @return Status of the main database load.
     */
    LoadStatus loadCardDatabases();

    /**
     * @brief Loads a single card database file into the given snapshot.
     * @param path Path to the database file.
     * @param data Snapshot to populate.
     * @return LoadStatus indicating success or failure.
     */
    LoadStatus loadCardDatabase(const QString &path, CardDatabaseData &data);

    /**
     * @brief Saves custom tokens to the user-defined custom database path.
     * @return True if the save was successful.
     */
    bool saveCustomTokensToFile();

signals:
    /** @brief Emitted when loading starts. */
    void loadingStarted();

    /** @brief Emitted when loading finishes successfully. */
    void loadingFinished();

    /** @brief Emitted when loading fails. */
    void loadingFailed();

    /**
     * @brief Emitted with the fully-built snapshot once background loading
     *        completes. The receiver swaps it into the live database.
     */
    void databaseDataReady(CardDatabaseData data);

    /**
     * @brief Emitted when new sets are discovered during loading.
     * @param numSets Number of new sets.
     * @param setNames Names of the discovered sets.
     */
    void newSetsFound(int numSets, const QStringList &setNames);

    /** @brief Emitted when all newly discovered sets have been enabled. */
    void allNewSetsEnabled();

private:
    /**
     * @brief Parses a single file into the given snapshot using the available parsers.
     * @param fileName Path to the database file.
     * @param data Snapshot to populate.
     * @return LoadStatus indicating success or failure.
     */
    LoadStatus loadFromFile(const QString &fileName, CardDatabaseData &data);

    /**
     * @brief Performs the actual load work synchronously on the calling thread.
     * @return Status of the main database load.
     */
    LoadStatus doLoadCardDatabases();

    /**
     * @brief Collects custom card database paths recursively.
     * @return Sorted list of file paths to custom databases.
     */
    [[nodiscard]] QStringList collectCustomDatabasePaths() const;

    /**
     * @brief Computes a hash identifying the current set of input database files.
     * @return Hash over the paths, sizes and modification times of all inputs.
     */
    [[nodiscard]] QByteArray computeSourceHash(const QStringList &customPaths) const;

    /**
     * @brief Path of the binary cache file derived from the main card database path.
     * @return Cache file path.
     */
    [[nodiscard]] QString cachePath() const;

    /**
     * @brief Attempts to populate the snapshot from the binary cache.
     * @param data Snapshot to populate.
     * @param sourceHash Pre-computed hash identifying the current input files.
     * @return True if a valid, up-to-date cache was read.
     */
    bool loadFromCache(CardDatabaseData &data, const QByteArray &sourceHash);

    /**
     * @brief Writes the snapshot to the binary cache for the current inputs.
     * @param data Snapshot to serialize.
     * @param sourceHash Pre-computed hash identifying the current input files.
     * @return True if the cache was written successfully.
     */
    bool saveToCache(const CardDatabaseData &data, const QByteArray &sourceHash);

private:
    CardDatabase *database;                         /**< Non-owning pointer to the target CardDatabase. */
    ICardDatabasePathProvider *pathProvider;        /**< Pointer to the path provider. */
    ICardSetPriorityController *priorityController; /**< Controller for reconstructing set state. */
    QList<ICardDatabaseParser *> availableParsers;  /**< List of available parsers for different formats. */

    QBasicMutex *loadFromFileMutex = new QBasicMutex();   /**< Mutex for single-file loading. */
    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(); /**< Mutex for reloading entire database. */
    bool initialLoadComplete = false;                     /**< Set after the first successful load. */
};

#endif // COCKATRICE_CARD_DATABASE_LOADER_H
