#ifndef SETTINGS_MIGRATION_H
#define SETTINGS_MIGRATION_H

#include <QString>

class SettingsMigration
{
public:
    static bool migrateSettingsFromGlobalIni(const QString &settingsPath);
    static bool migrateLegacySettings(const QString &settingsPath);

    /**
     * Returns true when at least one of the two migrations would still do work, i.e. when a
     * backup of the settings directory is worth taking before they run.
     */
    static bool isMigrationPending(const QString &settingsPath);

    /**
     * Copies the whole settings directory to a timestamped sibling folder, so a migration that
     * overwrites per-domain files can be undone by hand. Returns the backup path, or an empty
     * string when there was nothing to back up or the copy failed. Never aborts the migration.
     */
    static QString backupSettingsDirectory(const QString &settingsPath);
};

#endif // SETTINGS_MIGRATION_H
