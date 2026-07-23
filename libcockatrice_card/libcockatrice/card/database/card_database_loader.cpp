#include "card_database_loader.h"

#include "card_database.h"
#include "card_database_cache.h"
#include "parser/cockatrice_xml_3.h"
#include "parser/cockatrice_xml_4.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTime>

CardDatabaseLoader::CardDatabaseLoader(QObject *parent,
                                       CardDatabase *db,
                                       ICardDatabasePathProvider *_pathProvider,
                                       ICardPreferenceProvider *_preferenceProvider,
                                       ICardSetPriorityController *_priorityController)
    : QObject(parent), database(db), pathProvider(_pathProvider), priorityController(_priorityController)
{
    // instantiate available parsers here
    availableParsers << new CockatriceXml4Parser(_preferenceProvider, _priorityController);
    availableParsers << new CockatriceXml3Parser(_priorityController);

    // The load path parses into a snapshot and never emits per-card signals;
    // the finished snapshot is swapped into the live database on the GUI thread.

    // when SettingsCache's path changes, trigger reloads
    connect(pathProvider, &ICardDatabasePathProvider::cardDatabasePathChanged, this,
            &CardDatabaseLoader::loadCardDatabases);
}

CardDatabaseLoader::~CardDatabaseLoader()
{
    qDeleteAll(availableParsers);
    availableParsers.clear();
}

LoadStatus CardDatabaseLoader::loadFromFile(const QString &fileName, CardDatabaseData &data)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return FileError;
    }

    for (auto parser : availableParsers) {
        file.reset();
        if (parser->getCanParseFile(fileName, file)) {
            file.reset();
            parser->parseFileInto(file, data);
            return Ok;
        }
    }

    return Invalid;
}

LoadStatus CardDatabaseLoader::loadCardDatabase(const QString &path, CardDatabaseData &data)
{
    auto startTime = QTime::currentTime();
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty()) {
        QMutexLocker locker(loadFromFileMutex);
        tempLoadStatus = loadFromFile(path, data);
    }

    int msecs = startTime.msecsTo(QTime::currentTime());
    qCInfo(CardDatabaseLoadingLog) << "Loaded card database: Path =" << path << "Status =" << tempLoadStatus
                                   << "Cards =" << data.cards.size() << "Sets =" << data.sets.size()
                                   << QString("%1ms").arg(msecs);

    return tempLoadStatus;
}

LoadStatus CardDatabaseLoader::loadCardDatabases()
{
    return doLoadCardDatabases();
}

LoadStatus CardDatabaseLoader::doLoadCardDatabases()
{
    QMutexLocker locker(reloadDatabaseMutex);

    if (!database) {
        qCWarning(CardDatabaseLoadingLog) << "Loader has no database pointer";
        emit loadingFailed();
        return FileError;
    }
    emit loadingStarted();
    qCInfo(CardDatabaseLoadingLog) << "Card Database Loading Started";

    CardDatabaseData data;
    LoadStatus loadStatus = NotLoaded;

    // Try the binary cache first: a cache hit avoids re-parsing the (large) XML.
    if (loadFromCache(data)) {
        qCInfo(CardDatabaseLoadingLog) << "Loaded card database from binary cache";
        loadStatus = Ok;
    } else {
        loadStatus = loadCardDatabase(pathProvider->getCardDatabasePath(), data); // main card database
        if (loadStatus == Ok) {
            loadCardDatabase(pathProvider->getTokenDatabasePath(), data);       // tokens database
            loadCardDatabase(pathProvider->getSpoilerCardDatabasePath(), data); // spoilers database

            // find all custom card databases, recursively & following symlinks
            // then load them alphabetically
            const QStringList customPaths = collectCustomDatabasePaths();
            for (int i = 0; i < customPaths.size(); ++i) {
                const auto &p = customPaths.at(i);
                qCInfo(CardDatabaseLoadingLog) << "Loading Custom Set" << i << "(" << p << ")";
                loadCardDatabase(p, data);
            }

            saveToCache(data);
        }
    }

    // AFTER all the cards have been loaded: resolve the reverse-related tags
    // against the fully-built snapshot.
    database->refreshCachedReverseRelatedCards(data.cards);

    if (loadStatus == Ok) {
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Success";
        emit databaseDataReady(std::move(data));
        emit loadingFinished();
    } else {
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Failed";
        emit loadingFailed(); // bring up the settings dialog
    }

    return loadStatus;
}

QString CardDatabaseLoader::cachePath() const
{
    return pathProvider->getCardDatabasePath() + ".cache";
}

QByteArray CardDatabaseLoader::computeSourceHash() const
{
    // Hash over the paths, sizes and modification times of every input file so
    // the cache invalidates when any source changes. Cheap (no content read).
    QCryptographicHash hash(QCryptographicHash::Sha256);

    // Include the application version so parser changes automatically invalidate
    // old caches even when the XML files are byte-identical.
    hash.addData(QCoreApplication::applicationVersion().toUtf8());
    hash.addData(QByteArray(1, '\0'));

    const QStringList inputs = QStringList()
                               << pathProvider->getCardDatabasePath() << pathProvider->getTokenDatabasePath()
                               << pathProvider->getSpoilerCardDatabasePath() << collectCustomDatabasePaths();
    for (const QString &path : inputs) {
        QFileInfo info(path);
        if (info.exists()) {
            hash.addData(path.toUtf8());
            hash.addData(QByteArray(1, '\0'));
            hash.addData(QByteArray::number(info.size()));
            hash.addData(QByteArray(1, '\0'));
            hash.addData(QByteArray::number(info.lastModified().toSecsSinceEpoch()));
            hash.addData(QByteArray(1, '\0'));
        }
    }
    return hash.result();
}

bool CardDatabaseLoader::loadFromCache(CardDatabaseData &data)
{
    const QByteArray sourceHash = computeSourceHash();
    if (sourceHash.isEmpty()) {
        return false;
    }
    return CardDatabaseCache::read(cachePath(), data, sourceHash, priorityController);
}

void CardDatabaseLoader::saveToCache(const CardDatabaseData &data)
{
    const QByteArray sourceHash = computeSourceHash();
    if (sourceHash.isEmpty()) {
        return;
    }
    CardDatabaseCache::write(cachePath(), data, sourceHash);
}

QStringList CardDatabaseLoader::collectCustomDatabasePaths() const
{
    QDirIterator it(pathProvider->getCustomCardDatabasePath(), {"*.xml"}, QDir::Files,
                    QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    QStringList paths;
    while (it.hasNext()) {
        paths << it.next();
    }
    paths.sort();
    return paths;
}

bool CardDatabaseLoader::saveCustomTokensToFile()
{
    if (!database) {
        qCWarning(CardDatabaseLog) << "saveCustomTokensToFile: database pointer missing";
        return false;
    }

    QString fileName = pathProvider->getCustomCardDatabasePath() + "/" + CardSet::TOKENS_SETNAME + ".xml";

    SetNameMap tmpSets;
    CardSetPtr customTokensSet = database->getSet(CardSet::TOKENS_SETNAME);
    tmpSets.insert(CardSet::TOKENS_SETNAME, customTokensSet);

    CardNameMap tmpCards;
    for (const CardInfoPtr &card : database->cards) {
        if (card->getSets().contains(CardSet::TOKENS_SETNAME)) {
            tmpCards.insert(card->getName(), card);
        }
    }

    availableParsers.first()->saveToFile(FormatRulesNameMap(), tmpSets, tmpCards, fileName);
    return true;
}
