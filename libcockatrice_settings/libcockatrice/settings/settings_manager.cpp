#include "settings_manager.h"

SettingsManager::SettingsManager(const QString &settingPath, QObject *parent)
    : QObject(parent), settings(settingPath, QSettings::IniFormat)
{
}

void SettingsManager::setValue(const QVariant &value,
                               const QString &name,
                               const QString &group,
                               const QString &subGroup)
{
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    if (!subGroup.isEmpty()) {
        settings.beginGroup(subGroup);
    }

    settings.setValue(name, value);

    if (!subGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!group.isEmpty()) {
        settings.endGroup();
    }
}

void SettingsManager::deleteValue(const QString &name, const QString &group, const QString &subGroup)
{
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    if (!subGroup.isEmpty()) {
        settings.beginGroup(subGroup);
    }

    settings.remove(name);

    if (!subGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!group.isEmpty()) {
        settings.endGroup();
    }
}

QVariant SettingsManager::getValue(const QString &name, const QString &group, const QString &subGroup)
{
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    if (!subGroup.isEmpty()) {
        settings.beginGroup(subGroup);
    }

    QVariant value = settings.value(name);

    if (!subGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!group.isEmpty()) {
        settings.endGroup();
    }

    return value;
}

/**
 * Calls sync on the underlying QSettings object
 */
void SettingsManager::sync()
{
    settings.sync();
}