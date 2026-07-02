#include "path_settings.h"

#include <QDir>
#include <QFile>

PathSettings::PathSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "global.ini", "paths", QString(), parent)
{
}

QString PathSettings::getSafeConfigPath(const QString &configEntry, const QString &defaultPath) const
{
    QString tmp = getValue(configEntry).toString();
    // if the config settings is empty or refers to a not-existing folder,
    // ensure that the default path exists and return it
    if (tmp.isEmpty() || !QDir(tmp).exists()) {
        if (!QDir().mkpath(defaultPath)) {
            qCInfo(PathSettingsLog) << "[PathSettings] Could not create folder:" << defaultPath;
        }
        tmp = defaultPath;
    }
    return tmp;
}

QString PathSettings::getSafeConfigFilePath(const QString &configEntry, const QString &defaultPath) const
{
    QString tmp = getValue(configEntry).toString();
    // if the config settings is empty or refers to a not-existing file,
    // return the default path
    if (tmp.isEmpty() || !QFile::exists(tmp)) {
        tmp = defaultPath;
    }
    return tmp;
}

void PathSettings::recomputeCustomPicsPath()
{
    // this has never been exposed as a user-configurable setting
    if (picsPath.endsWith("/")) {
        customPicsPath = getSafeConfigPath("custompics", picsPath + "CUSTOM/");
    } else {
        customPicsPath = getSafeConfigPath("custompics", picsPath + "/CUSTOM/");
    }
}

void PathSettings::load(const QString &dataPath, const QString &cachePath)
{
    deckPath = getSafeConfigPath("decks", dataPath + "/decks/");
    filtersPath = getSafeConfigPath("filters", dataPath + "/filters/");
    replaysPath = getSafeConfigPath("replays", dataPath + "/replays/");
    themesPath = getSafeConfigPath("themes", dataPath + "/themes/");
    picsPath = getSafeConfigPath("pics", dataPath + "/pics/");
    redirectCachePath = getSafeConfigPath("redirects", cachePath + "/redirects/");
    recomputeCustomPicsPath();
    customCardDatabasePath = getSafeConfigPath("customsets", dataPath + "/customsets/");

    cardDatabasePath = getSafeConfigFilePath("carddatabase", dataPath + "/cards.xml");
    tokenDatabasePath = getSafeConfigFilePath("tokendatabase", dataPath + "/tokens.xml");
    spoilerDatabasePath = getSafeConfigFilePath("spoilerdatabase", dataPath + "/spoiler.xml");
}

void PathSettings::reset(const QString &dataPath, const QString &cachePath)
{
    auto settings = getSettings();
    settings.beginGroup("paths");
    settings.remove(""); // removes all keys in Path/*
    settings.endGroup();

    load(dataPath, cachePath);
}

/*void SettingsCache::loadPaths()
{
    QString dataPath = getDataPath();
    deckPath = getSafeConfigPath("paths/decks", dataPath + "/decks/");
    filtersPath = getSafeConfigPath("paths/filters", dataPath + "/filters/");
    replaysPath = getSafeConfigPath("paths/replays", dataPath + "/replays/");
    themesPath = getSafeConfigPath("paths/themes", dataPath + "/themes/");
    picsPath = getSafeConfigPath("paths/pics", dataPath + "/pics/");
    redirectCachePath = getSafeConfigPath("paths/redirects", getCachePath() + "/redirects/");
    // this has never been exposed as an user-configurable setting
    if (picsPath.endsWith("/")) {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "CUSTOM/");
    } else {
        customPicsPath = getSafeConfigPath("paths/custompics", picsPath + "/CUSTOM/");
    }
    customCardDatabasePath = getSafeConfigPath("paths/customsets", dataPath + "/customsets/");

    cardDatabasePath = getSafeConfigFilePath("paths/carddatabase", dataPath + "/cards.xml");
    tokenDatabasePath = getSafeConfigFilePath("paths/tokendatabase", dataPath + "/tokens.xml");
    spoilerDatabasePath = getSafeConfigFilePath("paths/spoilerdatabase", dataPath + "/spoiler.xml");
}

void SettingsCache::resetPaths()
{
    QStringList databasePaths{customCardDatabasePath, cardDatabasePath, spoilerDatabasePath, tokenDatabasePath};
    QString picsPath_ = picsPath;
    settings->remove("paths"); // removes all keys in paths/
    loadPaths();
    if (databasePaths !=
        QStringList{customCardDatabasePath, cardDatabasePath, spoilerDatabasePath, tokenDatabasePath}) {
        emit cardDatabasePathChanged();
        }
    if (picsPath_ != picsPath) {
        emit picsPathChanged();
    }
}*/

void PathSettings::setDeckPath(const QString &_deckPath)
{
    deckPath = _deckPath;
    setValue(deckPath, "decks");
}

void PathSettings::setFiltersPath(const QString &_filtersPath)
{
    filtersPath = _filtersPath;
    setValue(filtersPath, "filters");
}

void PathSettings::setReplaysPath(const QString &_replaysPath)
{
    replaysPath = _replaysPath;
    setValue(replaysPath, "replays");
}

void PathSettings::setThemesPath(const QString &_themesPath)
{
    themesPath = _themesPath;
    setValue(themesPath, "themes");
}

void PathSettings::setPicsPath(const QString &_picsPath)
{
    picsPath = _picsPath;
    setValue(picsPath, "pics");
    // get a new value for customPicsPath, currently derived from picsPath
    recomputeCustomPicsPath();
}

void PathSettings::setCustomCardDatabasePath(const QString &_customCardDatabasePath)
{
    customCardDatabasePath = _customCardDatabasePath;
    setValue(customCardDatabasePath, "customsets");
}

void PathSettings::setCardDatabasePath(const QString &_cardDatabasePath)
{
    cardDatabasePath = _cardDatabasePath;
    setValue(cardDatabasePath, "carddatabase");
}

void PathSettings::setSpoilerDatabasePath(const QString &_spoilerDatabasePath)
{
    spoilerDatabasePath = _spoilerDatabasePath;
    setValue(spoilerDatabasePath, "spoilerdatabase");
}

void PathSettings::setTokenDatabasePath(const QString &_tokenDatabasePath)
{
    tokenDatabasePath = _tokenDatabasePath;
    setValue(tokenDatabasePath, "tokendatabase");
}