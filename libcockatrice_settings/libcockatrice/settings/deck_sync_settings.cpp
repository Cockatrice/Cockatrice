#include "deck_sync_settings.h"

#include "libcockatrice/settings/settings_manager.h"

#include <QVariant>

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
