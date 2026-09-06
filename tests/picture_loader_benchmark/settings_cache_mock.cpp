/*
 * Minimal SettingsCache implementation for the picture loader benchmark.
 *
 * The picture loader code insists on reading every URL, cache and path via
 * SettingsCache::instance(). Compiling the real client SettingsCache would drag
 * in the network update-checker graph, so instead we provide the SettingsCache
 * member functions it actually uses, backed by the real libcockatrice_settings
 * manager classes pointed at a sandboxed settings directory.
 *
 * The mock must be named SettingsCache: PathsSettings declares
 * `friend class SettingsCache`, which is the only way to construct it.
 *
 * Paths resolved through QStandardPaths are redirected into the sandbox by
 * setting the XDG_* environment variables before the first instance() call.
 */

#include "client/settings/cache_settings.h"
#include "interface/card_picture_loader/card_picture_loader_cache_method.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <utility>

QString SettingsCache::getDataPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString SettingsCache::getSettingsPath()
{
    return getDataPath() + "/settings/";
}

QString SettingsCache::getCachePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QString SettingsCache::getNetworkCachePath() const
{
    return getCachePath() + "/downloaded/";
}

QString SettingsCache::getCustomCardDatabasePath() const
{
    return paths().getCustomCardDatabasePath();
}

QString SettingsCache::getCardDatabasePath() const
{
    return paths().getCardDatabasePath();
}

QString SettingsCache::getSpoilerCardDatabasePath() const
{
    return paths().getSpoilerCardDatabasePath();
}

QString SettingsCache::getTokenDatabasePath() const
{
    return paths().getTokenDatabasePath();
}

int SettingsCache::getKeepAlive() const
{
    return 0;
}

int SettingsCache::getTimeOut() const
{
    return 0;
}

bool SettingsCache::getNotifyAboutUpdates() const
{
    return false;
}

void SettingsCache::setKnownMissingFeatures(const QString & /*_knownMissingFeatures*/)
{
}

QString SettingsCache::getKnownMissingFeatures()
{
    return QString();
}

QString SettingsCache::getClientID()
{
    return QString();
}

DownloadSettings &SettingsCache::downloads() const
{
    return *downloadSettings;
}

CacheStorageSettings &SettingsCache::cacheStorage() const
{
    return *cacheStorageSettings;
}

CardsDisplaySettings &SettingsCache::cardsDisplay() const
{
    return *cardsDisplaySettings;
}

PathsSettings &SettingsCache::paths() const
{
    return *pathsSettings;
}

SettingsCache::SettingsCache()
{
    settings = nullptr;
    isPortableBuild = false;

    const QString settingsPath = getSettingsPath();
    QDir().mkpath(settingsPath);

    downloadSettings = new DownloadSettings(settingsPath, this);
    cacheStorageSettings = new CacheStorageSettings(settingsPath, this);
    cardsDisplaySettings = new CardsDisplaySettings(settingsPath, this);
    pathsSettings = new PathsSettings(settingsPath, this);

    // Picture downloads enabled, default host list, network disk cache storage.
    downloadSettings->setPicDownload(true);
    downloadSettings->resetToDefaultURLs();
    cacheStorageSettings->setCardImageCacheMethod(
        static_cast<int>(CardPictureLoaderCacheMethod::CacheMethod::NETWORK_CACHE));

    loadPaths();
}

void SettingsCache::loadPaths()
{
    QString dataPath = getDataPath();
    QSettings pathsIni(getSettingsPath() + "paths.ini", QSettings::IniFormat);

    auto computePath = [&](const QString &key, const QString &defaultPath) -> QString {
        QString val = pathsIni.value("paths/" + key).toString();
        if (val.isEmpty() || !QDir(val).exists()) {
            if (!QDir().mkpath(defaultPath)) {
                qCInfo(SettingsCacheLog) << "[SettingsCache] Could not create folder:" << defaultPath;
            }
            val = defaultPath;
            pathsIni.setValue("paths/" + key, val);
        }
        return val;
    };

    auto computeFilePath = [&](const QString &key, const QString &defaultPath) -> QString {
        QString val = pathsIni.value("paths/" + key).toString();
        if (!QFile::exists(val) || val.isEmpty()) {
            val = defaultPath;
            pathsIni.setValue("paths/" + key, val);
        }
        return val;
    };

    computePath("decks", dataPath + "/decks/");
    computePath("filters", dataPath + "/filters/");
    computePath("replays", dataPath + "/replays/");
    computePath("themes", dataPath + "/themes/");
    computePath("pics", dataPath + "/pics/");
    computePath("redirects", getCachePath() + "/redirects/");

    // customPicsPath derived from picsPath
    QString picsPath = pathsIni.value("paths/pics").toString();
    if (picsPath.endsWith("/")) {
        computePath("customPics", picsPath + "CUSTOM/");
    } else {
        computePath("customPics", picsPath + "/CUSTOM/");
    }

    computePath("customSets", dataPath + "/customsets/");
    computeFilePath("cardDatabase", dataPath + "/cards.xml");
    computeFilePath("tokenDatabase", dataPath + "/tokens.xml");
    computeFilePath("spoilerDatabase", dataPath + "/spoiler.xml");
}

void SettingsCache::setThemeName(const QString &_themeName)
{
    if (themeName != _themeName) {
        themeName = _themeName;
        emit themeChanged();
    }
}

SettingsCache &SettingsCache::instance()
{
    static SettingsCache settingsCache;
    return settingsCache;
}