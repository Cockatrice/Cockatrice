#ifndef TABS_SETTINGS_H
#define TABS_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_tabs_settings_provider.h>

/**
 * @brief The tab the application selects after launch.
 *
 * The destination is a request: tabs that were not open before (Deck Editor, Server Room, …)
 * are opened as part of the startup flow. Destinations that require a server connection use the
 * intent system to satisfy their pre-conditions.
 */
enum StartupTab
{
    StartupTabHome,              ///< The Home tab
    StartupTabVisualDeckStorage, ///< The visual deck storage tab
    StartupTabDeckStorage,       ///< The deck storage tab
    StartupTabReplays,           ///< The game replays tab
    StartupTabDeckEditor,        ///< A fresh classic deck editor tab
    StartupTabVisualDeckEditor,  ///< A fresh visual deck editor tab
    StartupTabServer,            ///< The server lobby: connect and select the server tab
    StartupTabServerRoom         ///< A server room: connect and join the room by name
};

class TabsSettings : public SettingsManager, public ITabsSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] int getStartupTabIndex() const override;
    [[nodiscard]] QString getStartupServerHost() const override;
    [[nodiscard]] QString getStartupServerPort() const override;
    [[nodiscard]] QString getStartupRoomName() const override;
    [[nodiscard]] bool getTabVisualDeckStorageOpen() const override;
    [[nodiscard]] bool getTabServerOpen() const override;
    [[nodiscard]] bool getTabAccountOpen() const override;
    [[nodiscard]] bool getTabDeckStorageOpen() const override;
    [[nodiscard]] bool getTabReplaysOpen() const override;
    [[nodiscard]] bool getTabAdminOpen() const override;
    [[nodiscard]] bool getTabLogOpen() const override;
    [[nodiscard]] bool getTabReportOpen() const override;
    [[nodiscard]] bool getTabModerationOpen() const override;
    [[nodiscard]] bool getTabCardArtRulesOpen() const override;

    void setStartupTabIndex(int value);
    void setStartupServerHost(const QString &host);
    void setStartupServerPort(const QString &port);
    void setStartupRoomName(const QString &roomName);
    void setTabVisualDeckStorageOpen(bool value);
    void setTabServerOpen(bool value);
    void setTabAccountOpen(bool value);
    void setTabDeckStorageOpen(bool value);
    void setTabReplaysOpen(bool value);
    void setTabAdminOpen(bool value);
    void setTabLogOpen(bool value);
    void setTabReportOpen(bool value);
    void setTabModerationOpen(bool value);
    void setTabCardArtRulesOpen(bool value);

signals:
    void startupTabIndexChanged(int index);
    void startupServerHostChanged(const QString &host);
    void startupServerPortChanged(const QString &port);
    void startupRoomNameChanged(const QString &roomName);

public:
    explicit TabsSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    TabsSettings(const TabsSettings & /*other*/);
};

#endif // TABS_SETTINGS_H
