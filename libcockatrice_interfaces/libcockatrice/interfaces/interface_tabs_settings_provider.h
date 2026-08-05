#ifndef COCKATRICE_INTERFACE_TABS_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_TABS_SETTINGS_PROVIDER_H

class ITabsSettingsProvider
{
public:
    virtual ~ITabsSettingsProvider() = default;

    [[nodiscard]] virtual bool getTabVisualDeckStorageOpen() const = 0;
    [[nodiscard]] virtual bool getTabServerOpen() const = 0;
    [[nodiscard]] virtual bool getTabAccountOpen() const = 0;
    [[nodiscard]] virtual bool getTabDeckStorageOpen() const = 0;
    [[nodiscard]] virtual bool getTabReplaysOpen() const = 0;
    [[nodiscard]] virtual bool getTabAdminOpen() const = 0;
    [[nodiscard]] virtual bool getTabLogOpen() const = 0;
};

#endif // COCKATRICE_INTERFACE_TABS_SETTINGS_PROVIDER_H
