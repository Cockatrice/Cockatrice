#ifndef TABS_SETTINGS_H
#define TABS_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_tabs_settings_provider.h>

class TabsSettings : public SettingsManager, public ITabsSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getTabVisualDeckStorageOpen() const override;
    [[nodiscard]] bool getTabServerOpen() const override;
    [[nodiscard]] bool getTabAccountOpen() const override;
    [[nodiscard]] bool getTabDeckStorageOpen() const override;
    [[nodiscard]] bool getTabReplaysOpen() const override;
    [[nodiscard]] bool getTabAdminOpen() const override;
    [[nodiscard]] bool getTabLogOpen() const override;

    void setTabVisualDeckStorageOpen(bool value);
    void setTabServerOpen(bool value);
    void setTabAccountOpen(bool value);
    void setTabDeckStorageOpen(bool value);
    void setTabReplaysOpen(bool value);
    void setTabAdminOpen(bool value);
    void setTabLogOpen(bool value);

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit TabsSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    TabsSettings(const TabsSettings & /*other*/);
};

#endif // TABS_SETTINGS_H
