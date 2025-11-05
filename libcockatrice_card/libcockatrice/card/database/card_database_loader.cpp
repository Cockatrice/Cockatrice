#include "card_database_loader.h"

#include "card_database.h"
#include "parser/cockatrice_xml_3.h"
#include "parser/cockatrice_xml_4.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QTime>
#include <libcockatrice/settings/cache_settings.h>

CardDatabaseLoader::CardDatabaseLoader(QObject *parent, CardDatabase *db) : QObject(parent), database(db)
{
    // instantiate available parsers here and connect them to the database
    availableParsers << new CockatriceXml4Parser;
    availableParsers << new CockatriceXml3Parser;

    for (auto *p : availableParsers) {
        // connect parser outputs to the database adders
        connect(p, &ICardDatabaseParser::addCard, database, &CardDatabase::addCard, Qt::DirectConnection);
        connect(p, &ICardDatabaseParser::addSet, database, &CardDatabase::addSet, Qt::DirectConnection);
    }

    // when SettingsCache's path changes, trigger reloads
    connect(&SettingsCache::instance(), &SettingsCache::cardDatabasePathChanged, this,
            &CardDatabaseLoader::loadCardDatabases);
}

CardDatabaseLoader::~CardDatabaseLoader()
{
    qDeleteAll(availableParsers);
    availableParsers.clear();
}

LoadStatus CardDatabaseLoader::loadFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return FileError;
    }

    for (auto parser : availableParsers) {
        file.reset();
        if (parser->getCanParseFile(fileName, file)) {
            file.reset();
            parser->parseFile(file);
            return Ok;
        }
    }

    return Invalid;
}

LoadStatus CardDatabaseLoader::loadCardDatabase(const QString &path)
{
    auto startTime = QTime::currentTime();
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty()) {
        QMutexLocker locker(loadFromFileMutex);
        tempLoadStatus = loadFromFile(path);
    }

    int msecs = startTime.msecsTo(QTime::currentTime());
    qCInfo(CardDatabaseLoadingLog) << "Loaded card database: Path =" << path << "Status =" << tempLoadStatus
                                   << "Cards =" << (database ? database->cards.size() : 0)
                                   << "Sets =" << (database ? database->sets.size() : 0) << QString("%1ms").arg(msecs);

    return tempLoadStatus;
}

LoadStatus CardDatabaseLoader::loadCardDatabases()
{
    QMutexLocker locker(reloadDatabaseMutex);

    if (!database) {
        qCWarning(CardDatabaseLoadingLog) << "Loader has no database pointer";
        emit loadingFailed();
        return FileError;
    }
    emit loadingStarted();
    qCInfo(CardDatabaseLoadingLog) << "Card Database Loading Started";

    database->clear(); // remove old db

    LoadStatus loadStatus =
        loadCardDatabase(SettingsCache::instance().getCardDatabasePath());    // load main card database
    loadCardDatabase(SettingsCache::instance().getTokenDatabasePath());       // load tokens database
    loadCardDatabase(SettingsCache::instance().getSpoilerCardDatabasePath()); // load spoilers database

    // find all custom card databases, recursively & following symlinks
    // then load them alphabetically
    const QStringList customPaths = collectCustomDatabasePaths();
    for (int i = 0; i < customPaths.size(); ++i) {
        const auto &p = customPaths.at(i);
        qCInfo(CardDatabaseLoadingLog) << "Loading Custom Set" << i << "(" << p << ")";
        loadCardDatabase(p);
    }

    // AFTER all the cards have been loaded

    // resolve the reverse-related tags

    database->refreshCachedReverseRelatedCards();

    if (loadStatus == Ok) {
        database->checkUnknownSets(); // update deck editors, etc
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Success";
        emit loadingFinished();
    } else {
        qCInfo(CardDatabaseLoadingSuccessOrFailureLog) << "Card Database Loading Failed";
        emit loadingFailed(); // bring up the settings dialog
    }

    return loadStatus;
}

QStringList CardDatabaseLoader::collectCustomDatabasePaths() const
{
    QDirIterator it(SettingsCache::instance().getCustomCardDatabasePath(), {"*.xml"}, QDir::Files,
                    QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    QStringList paths;
    while (it.hasNext())
        paths << it.next();
    paths.sort();
    return paths;
}

bool CardDatabaseLoader::saveCustomTokensToFile()
{
    if (!database) {
        qCWarning(CardDatabaseLog) << "saveCustomTokensToFile: database pointer missing";
        return false;
    }

    QString fileName = SettingsCache::instance().getCustomCardDatabasePath() + "/" + CardSet::TOKENS_SETNAME + ".xml";

    SetNameMap tmpSets;
    CardSetPtr customTokensSet = database->getSet(CardSet::TOKENS_SETNAME);
    tmpSets.insert(CardSet::TOKENS_SETNAME, customTokensSet);

    CardNameMap tmpCards;
    for (const CardInfoPtr &card : database->cards) {
        if (card->getSets().contains(CardSet::TOKENS_SETNAME)) {
            tmpCards.insert(card->getName(), card);
        }
    }

    availableParsers.first()->saveToFile(tmpSets, tmpCards, fileName);
    return true;
}
