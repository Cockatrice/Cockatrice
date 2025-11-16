#include "settings_manager.h"

SettingsManager::SettingsManager(const QString &settingPath,
                                 const QString &_defaultGroup,
                                 const QString &_defaultSubGroup,
                                 QObject *parent)
    : QObject(parent), settings(settingPath, QSettings::IniFormat), defaultGroup(_defaultGroup),
      defaultSubGroup(_defaultSubGroup)
{
}

void SettingsManager::setValue(const QVariant &value, const QString &name)
{
    if (!defaultGroup.isEmpty()) {
        settings.beginGroup(defaultGroup);
    }

    if (!defaultSubGroup.isEmpty()) {
        settings.beginGroup(defaultSubGroup);
    }

    settings.setValue(name, value);

    if (!defaultSubGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!defaultGroup.isEmpty()) {
        settings.endGroup();
    }
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

void SettingsManager::deleteValue(const QString &name)
{
    if (!defaultGroup.isEmpty()) {
        settings.beginGroup(defaultGroup);
    }

    if (!defaultSubGroup.isEmpty()) {
        settings.beginGroup(defaultSubGroup);
    }

    settings.remove(name);

    if (!defaultSubGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!defaultGroup.isEmpty()) {
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

QVariant SettingsManager::getValue(const QString &name)
{
    if (!defaultGroup.isEmpty()) {
        settings.beginGroup(defaultGroup);
    }

    if (!defaultSubGroup.isEmpty()) {
        settings.beginGroup(defaultSubGroup);
    }

    QVariant value = settings.value(name);

    if (!defaultSubGroup.isEmpty()) {
        settings.endGroup();
    }

    if (!defaultGroup.isEmpty()) {
        settings.endGroup();
    }

    return value;
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