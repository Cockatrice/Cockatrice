#ifndef COCKATRICE_CARD_DATABASE_LOADER_H
#define COCKATRICE_CARD_DATABASE_LOADER_H

#include <QBasicMutex>
#include <QList>
#include <QLoggingCategory>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_card_preference_provider.h>

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
                                ICardPreferenceProvider *preferenceProvider);

    /** @brief Destructor cleans up allocated parsers. */
    ~CardDatabaseLoader() override;

public slots:
    /**
     * @brief Loads all configured card databases.
     * @return Status of the main database load.
     */
    LoadStatus loadCardDatabases();

    /**
     * @brief Loads a single card database file.
     * @param path Path to the database file.
     * @return LoadStatus indicating success or failure.
     */
    LoadStatus loadCardDatabase(const QString &path);

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
     * @brief Emitted when new sets are discovered during loading.
     * @param numSets Number of new sets.
     * @param setNames Names of the discovered sets.
     */
    void newSetsFound(int numSets, const QStringList &setNames);

    /** @brief Emitted when all newly discovered sets have been enabled. */
    void allNewSetsEnabled();

private:
    /**
     * @brief Loads a database from a single file using the available parsers.
     * @param fileName Path to the database file.
     * @return LoadStatus indicating success or failure.
     */
    LoadStatus loadFromFile(const QString &fileName);

    /**
     * @brief Collects custom card database paths recursively.
     * @return Sorted list of file paths to custom databases.
     */
    [[nodiscard]] QStringList collectCustomDatabasePaths() const;

private:
    CardDatabase *database;                        /**< Non-owning pointer to the target CardDatabase. */
    ICardDatabasePathProvider *pathProvider;       /**< Pointer to the path provider. */
    QList<ICardDatabaseParser *> availableParsers; /**< List of available parsers for different formats. */

    QBasicMutex *loadFromFileMutex = new QBasicMutex();   /**< Mutex for single-file loading. */
    QBasicMutex *reloadDatabaseMutex = new QBasicMutex(); /**< Mutex for reloading entire database. */
};

#endif // COCKATRICE_CARD_DATABASE_LOADER_H
