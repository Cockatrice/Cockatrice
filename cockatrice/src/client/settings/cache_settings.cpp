#include "cache_settings.h"

#include "../network/update/client/release_channel.h"
#include "card_counter_settings.h"
#include "shortcuts_settings.h"
#include "version_string.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGlobalStatic>
#include <QSettings>
#include <QStandardPaths>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/card_database_settings.h>
#include <libcockatrice/settings/card_override_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/chat_settings.h>
#include <libcockatrice/settings/debug_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/game_filters_settings.h>
#include <libcockatrice/settings/game_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/layouts_settings.h>
#include <libcockatrice/settings/message_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/recents_settings.h>
#include <libcockatrice/settings/servers_settings.h>
#include <libcockatrice/settings/settings_migration.h>
#include <libcockatrice/settings/sound_settings.h>
#include <libcockatrice/settings/tabs_settings.h>
#include <libcockatrice/settings/updates_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <utility>

Q_GLOBAL_STATIC(SettingsCache, settingsCache)

SettingsCache &SettingsCache::instance()
{
    return *settingsCache; // returns a QT managed singleton reference
}

QString SettingsCache::getDataPath()
{
    if (isPortableBuild) {
        return qApp->applicationDirPath() + "/data";
    } else {
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }
}

QString SettingsCache::getSettingsPath()
{
    return getDataPath() + "/settings/";
}

QString SettingsCache::getCachePath() const
{
    if (isPortableBuild) {
        return qApp->applicationDirPath() + "/cache";
    } else {
        return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }
}

QString SettingsCache::getNetworkCachePath() const
{
    return getCachePath() + "/downloaded/";
}

QString SettingsCache::getSafeConfigPath(QString configEntry, QString defaultPath) const
{
    QString tmp = settings->value(configEntry).toString();
    // if the config settings is empty or refers to a not-existing folder,
    // ensure that the defaut path exists and return it
    if (tmp.isEmpty() || !QDir(tmp).exists()) {
        if (!QDir().mkpath(defaultPath)) {
            qCInfo(SettingsCacheLog) << "[SettingsCache] Could not create folder:" << defaultPath;
        }
        tmp = defaultPath;
    }
    return tmp;
}

QString SettingsCache::getSafeConfigFilePath(QString configEntry, QString defaultPath) const
{
    QString tmp = settings->value(configEntry).toString();
    // if the config settings is empty or refers to a not-existing file,
    // return the default Path
    if (!QFile::exists(tmp) || tmp.isEmpty()) {
        tmp = std::move(defaultPath);
    }
    return tmp;
}

SettingsCache::SettingsCache()
{
    // first, figure out if we are running in portable mode
    isPortableBuild = QFile::exists(qApp->applicationDirPath() + "/portable.dat");
    if (isPortableBuild) {
        qCInfo(SettingsCacheLog) << "Portable mode enabled";
    }

    QString settingsPath = getSettingsPath();

    settings = new QSettings(settingsPath + "global.ini", QSettings::IniFormat, this);

    // Migrate from legacy NativeFormat settings to per-class INI files (runs at most once)
    if (!isPortableBuild) {
        SettingsMigration::migrateLegacySettings(settingsPath);
    }

    // Migrate settings from global.ini to per-class INI files (runs at most once)
    SettingsMigration::migrateSettingsFromGlobalIni(settingsPath);

    shortcutsSettings = new ShortcutsSettings(settingsPath, this);
    cardDatabaseSettings = new CardDatabaseSettings(settingsPath, this);
    serversSettings = new ServersSettings(settingsPath, this);
    messageSettings = new MessageSettings(settingsPath, this);
    gameFiltersSettings = new GameFiltersSettings(settingsPath, this);
    layoutsSettings = new LayoutsSettings(settingsPath, this);
    downloadSettings = new DownloadSettings(settingsPath, this);
    recentsSettings = new RecentsSettings(settingsPath, this);
    cardOverrideSettings = new CardOverrideSettings(settingsPath, this);
    debugSettings = new DebugSettings(settingsPath, this);
    cardCounterSettings = new CardCounterSettings(settingsPath, this);

    tabsSettings = new TabsSettings(settingsPath, this);
    soundSettings = new SoundSettings(settingsPath, this);
    gameSettings = new GameSettings(settingsPath, this);
    chatSettings = new ChatSettings(settingsPath, this);
    cacheStorageSettings = new CacheStorageSettings(settingsPath, this);
    updatesSettings = new UpdatesSettings(settingsPath, this);
    personalSettings = new PersonalSettings(settingsPath, this);
    cardsDisplaySettings = new CardsDisplaySettings(settingsPath, this);
    interfaceSettings = new InterfaceSettings(settingsPath, this);
    pathsSettings = new PathsSettings(settingsPath, this);
    visualDeckStorageSettings = new VisualDeckStorageSettings(settingsPath, this);

    // Forward ICardDatabasePathProvider signal from PathsSettings
    connect(pathsSettings, &PathsSettings::cardDatabasePathChanged, this,
            &ICardDatabasePathProvider::cardDatabasePathChanged);

    // updates - don't reorder them or their index in the settings won't match
    // append channels one by one, or msvc will add them in the wrong order.
    releaseChannels << new StableReleaseChannel();
    releaseChannels << new BetaReleaseChannel();

    themeName = personalSettings->getThemeName();

    loadPaths();
}

void SettingsCache::setThemeName(const QString &_themeName)
{
    themeName = _themeName;
    personalSettings->setThemeName(themeName);
    emit themeChanged();
}

QStringList SettingsCache::getCountries() const
{
    static const QStringList countries = {
        "ad", "ae", "af", "ag", "ai", "al", "am", "ao", "aq", "ar", "as", "at", "au", "aw", "ax", "az", "ba", "bb",
        "bd", "be", "bf", "bg", "bh", "bi", "bj", "bl", "bm", "bn", "bo", "bq", "br", "bs", "bt", "bv", "bw", "by",
        "bz", "ca", "cc", "cd", "cf", "cg", "ch", "ci", "ck", "cl", "cm", "cn", "co", "cr", "cu", "cv", "cw", "cx",
        "cy", "cz", "de", "dj", "dk", "dm", "do", "dz", "ec", "ee", "eg", "eh", "er", "es", "et", "eu", "fi", "fj",
        "fk", "fm", "fo", "fr", "ga", "gb", "gd", "ge", "gf", "gg", "gh", "gi", "gl", "gm", "gn", "gp", "gq", "gr",
        "gs", "gt", "gu", "gw", "gy", "hk", "hm", "hn", "hr", "ht", "hu", "id", "ie", "il", "im", "in", "io", "iq",
        "ir", "is", "it", "je", "jm", "jo", "jp", "ke", "kg", "kh", "ki", "km", "kn", "kp", "kr", "kw", "ky", "kz",
        "la", "lb", "lc", "li", "lk", "lr", "ls", "lt", "lu", "lv", "ly", "ma", "mc", "md", "me", "mf", "mg", "mh",
        "mk", "ml", "mm", "mn", "mo", "mp", "mq", "mr", "ms", "mt", "mu", "mv", "mw", "mx", "my", "mz", "na", "nc",
        "ne", "nf", "ng", "ni", "nl", "no", "np", "nr", "nu", "nz", "om", "pa", "pe", "pf", "pg", "ph", "pk", "pl",
        "pm", "pn", "pr", "ps", "pt", "pw", "py", "qa", "re", "ro", "rs", "ru", "rw", "sa", "sb", "sc", "sd", "se",
        "sg", "sh", "si", "sj", "sk", "sl", "sm", "sn", "so", "sr", "ss", "st", "sv", "sx", "sy", "sz", "tc", "td",
        "tf", "tg", "th", "tj", "tk", "tl", "tm", "tn", "to", "tr", "tt", "tv", "tw", "tz", "ua", "ug", "um", "us",
        "uy", "uz", "va", "vc", "ve", "vg", "vi", "vn", "vu", "wf", "ws", "xk", "ye", "yt", "za", "zm", "zw"};

    return countries;
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
        computePath("custompics", picsPath + "CUSTOM/");
    } else {
        computePath("custompics", picsPath + "/CUSTOM/");
    }

    computePath("customsets", dataPath + "/customsets/");
    computeFilePath("carddatabase", dataPath + "/cards.xml");
    computeFilePath("tokendatabase", dataPath + "/tokens.xml");
    computeFilePath("spoilerdatabase", dataPath + "/spoiler.xml");
}

void SettingsCache::resetPaths()
{
    QStringList databasePaths{pathsSettings->getCustomCardDatabasePath(), pathsSettings->getCardDatabasePath(),
                              pathsSettings->getSpoilerCardDatabasePath(), pathsSettings->getTokenDatabasePath()};
    QString picsPath_ = pathsSettings->getPicsPath();

    QSettings pathsIni(getSettingsPath() + "paths.ini", QSettings::IniFormat);
    pathsIni.remove("paths");

    loadPaths();

    if (databasePaths != QStringList{pathsSettings->getCustomCardDatabasePath(), pathsSettings->getCardDatabasePath(),
                                     pathsSettings->getSpoilerCardDatabasePath(),
                                     pathsSettings->getTokenDatabasePath()}) {
        emit cardDatabasePathChanged();
    }
    if (picsPath_ != pathsSettings->getPicsPath()) {
        emit pathsSettings->picsPathChanged();
    }
}

// ICardDatabasePathProvider - delegate to pathsSettings
QString SettingsCache::getCustomCardDatabasePath() const
{
    return pathsSettings->getCustomCardDatabasePath();
}

QString SettingsCache::getCardDatabasePath() const
{
    return pathsSettings->getCardDatabasePath();
}

QString SettingsCache::getSpoilerCardDatabasePath() const
{
    return pathsSettings->getSpoilerCardDatabasePath();
}

QString SettingsCache::getTokenDatabasePath() const
{
    return pathsSettings->getTokenDatabasePath();
}

// INetworkSettingsProvider - delegate to sub-objects
int SettingsCache::getKeepAlive() const
{
    return personalSettings->getKeepAlive();
}

int SettingsCache::getTimeOut() const
{
    return personalSettings->getTimeOut();
}

bool SettingsCache::getNotifyAboutUpdates() const
{
    return updatesSettings->getNotifyAboutUpdates();
}

void SettingsCache::setKnownMissingFeatures(const QString &_knownMissingFeatures)
{
    interfaceSettings->setKnownMissingFeatures(_knownMissingFeatures);
}

QString SettingsCache::getKnownMissingFeatures()
{
    return interfaceSettings->getKnownMissingFeatures();
}

QString SettingsCache::getClientID()
{
    return personalSettings->getClientID();
}

// Release channels
ReleaseChannel *SettingsCache::getUpdateReleaseChannel() const
{
    return releaseChannels.at(qMax(0, updatesSettings->getUpdateReleaseChannelIndex()));
}

QList<ReleaseChannel *> SettingsCache::getUpdateReleaseChannels() const
{
    return releaseChannels;
}

int SettingsCache::getUpdateReleaseChannelIndex() const
{
    return updatesSettings->getUpdateReleaseChannelIndex();
}

// Sub-settings accessors
ShortcutsSettings &SettingsCache::shortcuts() const
{
    return *shortcutsSettings;
}

CardDatabaseSettings &SettingsCache::cardDatabase() const
{
    return *cardDatabaseSettings;
}

ServersSettings &SettingsCache::servers() const
{
    return *serversSettings;
}

MessageSettings &SettingsCache::messages() const
{
    return *messageSettings;
}

GameFiltersSettings &SettingsCache::gameFilters() const
{
    return *gameFiltersSettings;
}

LayoutsSettings &SettingsCache::layouts() const
{
    return *layoutsSettings;
}

DownloadSettings &SettingsCache::downloads() const
{
    return *downloadSettings;
}

RecentsSettings &SettingsCache::recents() const
{
    return *recentsSettings;
}

CardOverrideSettings &SettingsCache::cardOverrides() const
{
    return *cardOverrideSettings;
}

DebugSettings &SettingsCache::debug() const
{
    return *debugSettings;
}

CardCounterSettings &SettingsCache::cardCounters() const
{
    return *cardCounterSettings;
}

TabsSettings &SettingsCache::tabs() const
{
    return *tabsSettings;
}

SoundSettings &SettingsCache::sound() const
{
    return *soundSettings;
}

GameSettings &SettingsCache::game() const
{
    return *gameSettings;
}

ChatSettings &SettingsCache::chat() const
{
    return *chatSettings;
}

CacheStorageSettings &SettingsCache::cacheStorage() const
{
    return *cacheStorageSettings;
}

UpdatesSettings &SettingsCache::updates() const
{
    return *updatesSettings;
}

PersonalSettings &SettingsCache::personal() const
{
    return *personalSettings;
}

CardsDisplaySettings &SettingsCache::cardsDisplay() const
{
    return *cardsDisplaySettings;
}

InterfaceSettings &SettingsCache::interface() const
{
    return *interfaceSettings;
}

PathsSettings &SettingsCache::paths() const
{
    return *pathsSettings;
}

VisualDeckStorageSettings &SettingsCache::visualDeckStorage() const
{
    return *visualDeckStorageSettings;
}
