#include "deck_sync_settings.h"

#include "libcockatrice/settings/settings_manager.h"

#include <QVariant>
#include <QtGlobal>

DeckSyncSettings::DeckSyncSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "deck_sync.ini", "decksync", QString(), parent)
{
}

int DeckSyncSettings::getConflictDefault() const
{
    return getValue("conflictDefault", QString(), QString(), DeckSyncConflictAsk).toInt();
}

void DeckSyncSettings::setConflictDefault(int _conflictDefault)
{
    setValue(_conflictDefault, "conflictDefault");
    emit conflictDefaultChanged(_conflictDefault);
}

bool DeckSyncSettings::getAutoSync() const
{
    return getValue("autoSync", QString(), QString(), true).toBool();
}

void DeckSyncSettings::setAutoSync(bool _autoSync)
{
    setValue(_autoSync, "autoSync");
    emit autoSyncChanged(_autoSync);
}

int DeckSyncSettings::getSyncIntervalMinutes() const
{
    return qBound(1, getValue("syncIntervalMinutes", QString(), QString(), 5).toInt(), 60);
}

void DeckSyncSettings::setSyncIntervalMinutes(int _syncIntervalMinutes)
{
    setValue(_syncIntervalMinutes, "syncIntervalMinutes");
    emit syncIntervalMinutesChanged(_syncIntervalMinutes);
}
