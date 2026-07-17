/**
 * @file cache_settings.h
 * @ingroup Settings
 */
//! \todo Document this file.

#ifndef SETTINGSCACHE_H
#define SETTINGSCACHE_H

#include "shortcuts_settings.h"

#include <QLoggingCategory>
#include <QStringList>
#include <libcockatrice/interfaces/interface_card_database_path_provider.h>
#include <libcockatrice/interfaces/interface_network_settings_provider.h>
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
#include <libcockatrice/settings/sound_settings.h>
#include <libcockatrice/settings/tabs_settings.h>
#include <libcockatrice/settings/updates_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <libcockatrice/utility/macros.h>

inline Q_LOGGING_CATEGORY(SettingsCacheLog, "settings_cache");

class ReleaseChannel;

#define DEFAULT_LANG_NAME "English"
#define CLIENT_INFO_NOT_SET "notset"

#define DEFAULT_FONT_SIZE 12

class QSettings;
class CardCounterSettings;

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
    [[nodiscard]] QString getCustomCardDatabasePath() const override
    {
        return pathsSettings->getCustomCardDatabasePath();
    }
    [[nodiscard]] QString getCardDatabasePath() const override
    {
        return pathsSettings->getCardDatabasePath();
    }
    [[nodiscard]] QString getSpoilerCardDatabasePath() const override
    {
        return pathsSettings->getSpoilerCardDatabasePath();
    }
    [[nodiscard]] QString getTokenDatabasePath() const override
    {
        return pathsSettings->getTokenDatabasePath();
    }

    // INetworkSettingsProvider - delegate to sub-objects
    [[nodiscard]] int getKeepAlive() const override
    {
        return personalSettings->getKeepAlive();
    }
    [[nodiscard]] int getTimeOut() const override
    {
        return personalSettings->getTimeOut();
    }
    [[nodiscard]] bool getNotifyAboutUpdates() const override
    {
        return updatesSettings->getNotifyAboutUpdates();
    }
    void setKnownMissingFeatures(const QString &_knownMissingFeatures) override
    {
        interfaceSettings->setKnownMissingFeatures(_knownMissingFeatures);
    }
    [[nodiscard]] QString getKnownMissingFeatures() override
    {
        return interfaceSettings->getKnownMissingFeatures();
    }
    QString getClientID() override
    {
        return personalSettings->getClientID();
    }

    [[nodiscard]] QString getThemeName() const
    {
        return themeName;
    }

    [[nodiscard]] ReleaseChannel *getUpdateReleaseChannel() const
    {
        return releaseChannels.at(qMax(0, updatesSettings->getUpdateReleaseChannelIndex()));
    }
    [[nodiscard]] QList<ReleaseChannel *> getUpdateReleaseChannels() const
    {
        return releaseChannels;
    }
    [[nodiscard]] int getUpdateReleaseChannelIndex() const
    {
        return updatesSettings->getUpdateReleaseChannelIndex();
    }

    [[nodiscard]] QStringList getCountries() const;

    [[nodiscard]] ShortcutsSettings &shortcuts() const
    {
        return *shortcutsSettings;
    }
    [[nodiscard]] CardDatabaseSettings &cardDatabase() const
    {
        return *cardDatabaseSettings;
    }
    [[nodiscard]] ServersSettings &servers() const
    {
        return *serversSettings;
    }
    [[nodiscard]] MessageSettings &messages() const
    {
        return *messageSettings;
    }
    [[nodiscard]] GameFiltersSettings &gameFilters() const
    {
        return *gameFiltersSettings;
    }
    [[nodiscard]] LayoutsSettings &layouts() const
    {
        return *layoutsSettings;
    }
    [[nodiscard]] DownloadSettings &downloads() const
    {
        return *downloadSettings;
    }
    [[nodiscard]] RecentsSettings &recents() const
    {
        return *recentsSettings;
    }
    [[nodiscard]] CardOverrideSettings &cardOverrides() const
    {
        return *cardOverrideSettings;
    }
    [[nodiscard]] DebugSettings &debug() const
    {
        return *debugSettings;
    }
    [[nodiscard]] CardCounterSettings &cardCounters() const;
    [[nodiscard]] TabsSettings &tabs() const
    {
        return *tabsSettings;
    }
    [[nodiscard]] SoundSettings &sound() const
    {
        return *soundSettings;
    }
    [[nodiscard]] GameSettings &game() const
    {
        return *gameSettings;
    }
    [[nodiscard]] ChatSettings &chat() const
    {
        return *chatSettings;
    }
    [[nodiscard]] CacheStorageSettings &cacheStorage() const
    {
        return *cacheStorageSettings;
    }
    [[nodiscard]] UpdatesSettings &updates() const
    {
        return *updatesSettings;
    }
    [[nodiscard]] PersonalSettings &personal() const
    {
        return *personalSettings;
    }
    [[nodiscard]] CardsDisplaySettings &cardsDisplay() const
    {
        return *cardsDisplaySettings;
    }
    [[nodiscard]] InterfaceSettings &interface() const
    {
        return *interfaceSettings;
    }
    [[nodiscard]] PathsSettings &paths() const
    {
        return *pathsSettings;
    }
    [[nodiscard]] VisualDeckStorageSettings &visualDeckStorage() const
    {
        return *visualDeckStorageSettings;
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
