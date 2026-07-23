#ifndef UPDATES_SETTINGS_H
#define UPDATES_SETTINGS_H

#include "settings_manager.h"

#include <QDate>
#include <libcockatrice/interfaces/interface_updates_settings_provider.h>

class UpdatesSettings : public SettingsManager, public IUpdatesSettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getCheckUpdatesOnStartup() const override;
    [[nodiscard]] bool getStartupCardUpdateCheckPromptForUpdate() override;
    [[nodiscard]] bool getStartupCardUpdateCheckAlwaysUpdate() override;
    [[nodiscard]] int getCardUpdateCheckInterval() const override;
    [[nodiscard]] QDate getLastCardUpdateCheck() const override;
    [[nodiscard]] bool getCardUpdateCheckRequired() const override;
    [[nodiscard]] bool getAlwaysEnableNewSets() const override;
    [[nodiscard]] bool getNotifyAboutUpdates() const override;
    [[nodiscard]] bool getNotifyAboutNewVersion() const override;
    [[nodiscard]] int getUpdateReleaseChannelIndex() const override;

    void setCheckUpdatesOnStartup(bool value);
    void setStartupCardUpdateCheckPromptForUpdate(bool value);
    void setStartupCardUpdateCheckAlwaysUpdate(bool value);
    void setCardUpdateCheckInterval(int value);
    void setLastCardUpdateCheck(QDate value);
    void setAlwaysEnableNewSets(bool value);
    void setNotifyAboutUpdates(bool _notifyaboutupdate);
    void setNotifyAboutNewVersion(bool _notifyaboutnewversion);
    void setUpdateReleaseChannelIndex(int value);

#ifdef SETTINGS_UNIT_TEST
public:
#else
private:
#endif
    explicit UpdatesSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    UpdatesSettings(const UpdatesSettings & /*other*/);
};

#endif // UPDATES_SETTINGS_H
