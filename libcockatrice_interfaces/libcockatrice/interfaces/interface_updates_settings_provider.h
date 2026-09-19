#ifndef COCKATRICE_INTERFACE_UPDATES_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_UPDATES_SETTINGS_PROVIDER_H

#include <QDate>

class IUpdatesSettingsProvider
{
public:
    virtual ~IUpdatesSettingsProvider() = default;

    [[nodiscard]] virtual bool getCheckUpdatesOnStartup() const = 0;
    [[nodiscard]] virtual bool getStartupCardUpdateCheckPromptForUpdate() = 0;
    [[nodiscard]] virtual bool getStartupCardUpdateCheckAlwaysUpdate() = 0;
    [[nodiscard]] virtual int getCardUpdateCheckInterval() const = 0;
    [[nodiscard]] virtual QDate getLastCardUpdateCheck() const = 0;
    [[nodiscard]] virtual bool getCardUpdateCheckRequired() const = 0;
    [[nodiscard]] virtual bool getAlwaysEnableNewSets() const = 0;
    [[nodiscard]] virtual bool getNotifyAboutUpdates() const = 0;
    [[nodiscard]] virtual bool getNotifyAboutNewVersion() const = 0;
    [[nodiscard]] virtual int getUpdateReleaseChannelIndex() const = 0;
};

#endif // COCKATRICE_INTERFACE_UPDATES_SETTINGS_PROVIDER_H
