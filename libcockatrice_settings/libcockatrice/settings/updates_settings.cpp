#include "updates_settings.h"

#include <QDateTime>

UpdatesSettings::UpdatesSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "updates.ini", "updates", QString(), parent)
{
}

bool UpdatesSettings::getCheckUpdatesOnStartup() const
{
    return getValue("startupUpdateCheck", QString(), QString(), true).toBool();
}

bool UpdatesSettings::getStartupCardUpdateCheckPromptForUpdate()
{
    return getValue("startupCardUpdateCheckPromptForUpdate", QString(), QString(), true).toBool();
}

bool UpdatesSettings::getStartupCardUpdateCheckAlwaysUpdate()
{
    return getValue("startupCardUpdateCheckAlwaysUpdate", QString(), QString(), false).toBool();
}

int UpdatesSettings::getCardUpdateCheckInterval() const
{
    return getValue("cardUpdateCheckInterval", QString(), QString(), 7).toInt();
}

QDate UpdatesSettings::getLastCardUpdateCheck() const
{
    return getValue("lastCardUpdateCheck", QString(), QString(), QDateTime::currentDateTime().date()).toDate();
}

bool UpdatesSettings::getCardUpdateCheckRequired() const
{
    return getLastCardUpdateCheck().daysTo(QDateTime::currentDateTime().date()) >= getCardUpdateCheckInterval() &&
           getLastCardUpdateCheck() != QDateTime::currentDateTime().date();
}

bool UpdatesSettings::getAlwaysEnableNewSets() const
{
    return getValue("alwaysEnableNewSets", QString(), QString(), false).toBool();
}

bool UpdatesSettings::getNotifyAboutUpdates() const
{
    return getValue("updateNotification", QString(), QString(), true).toBool();
}

bool UpdatesSettings::getNotifyAboutNewVersion() const
{
    return getValue("newVersionNotification", QString(), QString(), true).toBool();
}

int UpdatesSettings::getUpdateReleaseChannelIndex() const
{
    return getValue("updateReleaseChannel", QString(), QString(), 0).toInt();
}

void UpdatesSettings::setCheckUpdatesOnStartup(bool value)
{
    setValue(value, "startupUpdateCheck");
}

void UpdatesSettings::setStartupCardUpdateCheckPromptForUpdate(bool value)
{
    setValue(value, "startupCardUpdateCheckPromptForUpdate");
}

void UpdatesSettings::setStartupCardUpdateCheckAlwaysUpdate(bool value)
{
    setValue(value, "startupCardUpdateCheckAlwaysUpdate");
}

void UpdatesSettings::setCardUpdateCheckInterval(int value)
{
    setValue(value, "cardUpdateCheckInterval");
}

void UpdatesSettings::setLastCardUpdateCheck(QDate value)
{
    setValue(value, "lastCardUpdateCheck");
}

void UpdatesSettings::setAlwaysEnableNewSets(bool value)
{
    setValue(value, "alwaysEnableNewSets");
}

void UpdatesSettings::setNotifyAboutUpdates(bool _notifyaboutupdate)
{
    setValue(_notifyaboutupdate, "updateNotification");
}

void UpdatesSettings::setNotifyAboutNewVersion(bool _notifyaboutnewversion)
{
    setValue(_notifyaboutnewversion, "newVersionNotification");
}

void UpdatesSettings::setUpdateReleaseChannelIndex(int value)
{
    setValue(value, "updateReleaseChannel");
}
