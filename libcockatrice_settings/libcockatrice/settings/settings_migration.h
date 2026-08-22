#ifndef SETTINGS_MIGRATION_H
#define SETTINGS_MIGRATION_H

#include <QString>

class SettingsMigration
{
public:
    static bool migrateSettingsFromGlobalIni(const QString &settingsPath);
    static bool migrateLegacySettings(const QString &settingsPath);
};

#endif // SETTINGS_MIGRATION_H
