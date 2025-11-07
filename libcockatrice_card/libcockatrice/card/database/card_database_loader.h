/**
 * @file card_database_loader.h
 * @ingroup CardDatabase
 * @brief The CardDatabaseLoader is responsible for populating the card database from files on disk.
 */

#ifndef COCKATRICE_CARD_DATABASE_LOADER_H
#define COCKATRICE_CARD_DATABASE_LOADER_H

#include <QBasicMutex>
#include <QList>
#include <QLoggingCategory>
#include <QObject>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>

inline Q_LOGGING_CATEGORY(CardDatabaseLoadingLog, "card_database.loading");
inline Q_LOGGING_CATEGORY(CardDatabaseLoadingSuccessOrFailureLog, "card_database.loading.success_or_failure");

class CardDatabase;
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

class CardDatabaseLoader : public QObject
{
    Q_OBJECT
public:
    explicit CardDatabaseLoader(QObject *parent, CardDatabase *db, ICardDatabasePathProvider *pathProvider);
    ~CardDatabaseLoader() override;

public slots:
    LoadStatus loadCardDatabases();                   // discover & load the configured databases
    LoadStatus loadCardDatabase(const QString &path); // load a single file
    bool saveCustomTokensToFile();                    // write tokens to custom DB path

signals:
    void loadingStarted();
    void loadingFinished();
    void loadingFailed();
    void newSetsFound(int numSets, const QStringList &setNames);
    void allNewSetsEnabled();

private:
    LoadStatus loadFromFile(const QString &fileName); // internal helper
    QStringList collectCustomDatabasePaths() const;

    CardDatabase *database; // non-owning pointer to the container

    ICardDatabasePathProvider *pathProvider; // pointer to the implementation providing the paths

    // parsers
    QList<ICardDatabaseParser *> availableParsers;

    QBasicMutex *loadFromFileMutex = new QBasicMutex();
    QBasicMutex *reloadDatabaseMutex = new QBasicMutex();
};

#endif // COCKATRICE_CARD_DATABASE_LOADER_H
