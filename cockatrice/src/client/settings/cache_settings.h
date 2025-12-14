/**
 * @file cache_settings.h
 * @ingroup Settings
 */
//! \todo Document this file.

#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include <QLoggingCategory>
#include <QStringList>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>
#include <libcockatrice/utility/macros.h>

inline Q_LOGGING_CATEGORY(SettingsCacheLog, "settings_cache");

class ReleaseChannel;

#define DEFAULT_LANG_NAME "English"
#define CLIENT_INFO_NOT_SET "notset"

#define DEFAULT_FONT_SIZE 12

class CacheStorageSettings;
class CardCounterSettings;
class CardDatabaseSettings;
class CardOverrideSettings;
class CardsDisplaySettings;
class ChatSettings;
class CommanderBracketSettings;
class DebugSettings;
class DownloadSettings;
class GameFiltersSettings;
class GameSettings;
class InterfaceSettings;
class LayoutsSettings;
class MessageSettings;
class PathsSettings;
class PersonalSettings;
class RecentsSettings;
class ServersSettings;
class ShortcutsSettings;
class SoundSettings;
class TabsSettings;
class UpdatesSettings;
class VisualDeckStorageSettings;
class QSettings;

class SettingsCache : public ICardDatabasePathProvider, public INetworkSettingsProvider
{
    Q_OBJECT

signals:
    void themeChanged();

private:
    QSettings *settings;
    ShortcutsSettings *shortcutsSettings;
    CardDatabaseSettings *cardDatabaseSettings;
    ServersSettings *serversSettings;
    MessageSettings *messageSettings;
    GameFiltersSettings *gameFiltersSettings;
    LayoutsSettings *layoutsSettings;
    DownloadSettings *downloadSettings;
    RecentsSettings *recentsSettings;
    CardOverrideSettings *cardOverrideSettings;
    DebugSettings *debugSettings;
    CardCounterSettings *cardCounterSettings;
    TabsSettings *tabsSettings;
    SoundSettings *soundSettings;
    GameSettings *gameSettings;
    ChatSettings *chatSettings;
    CacheStorageSettings *cacheStorageSettings;
    UpdatesSettings *updatesSettings;
    PersonalSettings *personalSettings;
    CardsDisplaySettings *cardsDisplaySettings;
    InterfaceSettings *interfaceSettings;
    PathsSettings *pathsSettings;
    VisualDeckStorageSettings *visualDeckStorageSettings;
    CommanderBracketSettings *commanderBracketSettings;

    QString themeName;

    QList<ReleaseChannel *> releaseChannels;
    bool isPortableBuild;

    [[nodiscard]] QString getSafeConfigPath(QString configEntry, QString defaultPath) const;
    [[nodiscard]] QString getSafeConfigFilePath(QString configEntry, QString defaultPath) const;
    void loadPaths();

public:
    SettingsCache();
    QString getDataPath();
    QString getSettingsPath();
    [[nodiscard]] QString getCachePath() const;
    [[nodiscard]] QString getNetworkCachePath() const;

    // ICardDatabasePathProvider - delegate to pathsSettings
    [[nodiscard]] QString getCustomCardDatabasePath() const override;
    [[nodiscard]] QString getCardDatabasePath() const override;
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override;
    [[nodiscard]] QString getTokenDatabasePath() const override;

    // INetworkSettingsProvider - delegate to sub-objects
    [[nodiscard]] int getKeepAlive() const override;
    [[nodiscard]] int getTimeOut() const override;
    [[nodiscard]] bool getNotifyAboutUpdates() const override;
    void setKnownMissingFeatures(const QString &_knownMissingFeatures) override;
    [[nodiscard]] QString getKnownMissingFeatures() override;
    QString getClientID() override;

    [[nodiscard]] QString getThemeName() const
    {
        return themeName;
    }

    [[nodiscard]] ReleaseChannel *getUpdateReleaseChannel() const;
    [[nodiscard]] QList<ReleaseChannel *> getUpdateReleaseChannels() const;
    [[nodiscard]] int getUpdateReleaseChannelIndex() const;

    [[nodiscard]] QStringList getCountries() const;

    [[nodiscard]] ShortcutsSettings &shortcuts() const;
    [[nodiscard]] CardDatabaseSettings &cardDatabase() const;
    [[nodiscard]] ServersSettings &servers() const;
    [[nodiscard]] MessageSettings &messages() const;
    [[nodiscard]] GameFiltersSettings &gameFilters() const;
    [[nodiscard]] LayoutsSettings &layouts() const;
    [[nodiscard]] DownloadSettings &downloads() const;
    [[nodiscard]] RecentsSettings &recents() const;
    [[nodiscard]] CardOverrideSettings &cardOverrides() const;
    [[nodiscard]] DebugSettings &debug() const;
    [[nodiscard]] CardCounterSettings &cardCounters() const;
    [[nodiscard]] TabsSettings &tabs() const;
    [[nodiscard]] SoundSettings &sound() const;
    [[nodiscard]] GameSettings &game() const;
    [[nodiscard]] ChatSettings &chat() const;
    [[nodiscard]] CacheStorageSettings &cacheStorage() const;
    [[nodiscard]] UpdatesSettings &updates() const;
    [[nodiscard]] PersonalSettings &personal() const;
    [[nodiscard]] CardsDisplaySettings &cardsDisplay() const;
    [[nodiscard]] InterfaceSettings &interface() const;
    [[nodiscard]] PathsSettings &paths() const;
    [[nodiscard]] VisualDeckStorageSettings &visualDeckStorage() const;
    [[nodiscard]] CommanderBracketSettings &commanderBrackets() const
    {
        return *commanderBracketSettings;
    }

    [[nodiscard]] bool getIsPortableBuild() const
    {
        return isPortableBuild;
    }

    static SettingsCache &instance();
    void resetPaths();

public slots:
    void setThemeName(const QString &_themeName);
};
#endif
