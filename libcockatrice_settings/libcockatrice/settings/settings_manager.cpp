#include "settings_manager.h"

SettingsManager::SettingsManager(const QString &_settingPath,
                                 const QString &_defaultGroup,
                                 const QString &_defaultSubGroup,
                                 QObject *parent)
    : QObject(parent), settingPath(_settingPath), defaultGroup(_defaultGroup), defaultSubGroup(_defaultSubGroup)
{
}

QSettings SettingsManager::getSettings() const
{
    // Do not store the QSettings instance in a field, as that is not threadsafe (see #6747)
    return QSettings(settingPath, QSettings::IniFormat);
}

void SettingsManager::setValue(const QVariant &value, const QString &name)
{
    auto settings = getSettings();

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
    auto settings = getSettings();

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
    auto settings = getSettings();

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
    auto settings = getSettings();

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

QVariant SettingsManager::getValue(const QString &name) const
{
    auto settings = getSettings();

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

QVariant SettingsManager::getValue(const QString &name, const QString &group, const QString &subGroup) const
{
    auto settings = getSettings();

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

void SettingsManager::batchWrite(std::function<void(QSettings &)> batchWriteFunction)
{
    auto settings = getSettings();
    settings.setAtomicSyncRequired(false);
    batchWriteFunction(settings);
    settings.sync(); // single flush
    settings.setAtomicSyncRequired(true);
}

/**
 * Calls sync on the underlying QSettings object
 */
void SettingsManager::sync()
{
    auto settings = getSettings();

    settings.sync();
}
