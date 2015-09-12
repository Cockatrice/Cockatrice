#include "settingsmanager.h"

SettingsManager::SettingsManager(QString settingPath, QObject *parent)
    : QObject(parent),
      settings(settingPath, QSettings::IniFormat)
{
}

void SettingsManager::setValue(QVariant value, QString name, QString group, QString subGroup)
{
    if(!group.isEmpty())
        settings.beginGroup(group);

    if(!subGroup.isEmpty())
        settings.beginGroup(subGroup);

    settings.setValue(name, value);

    if(!subGroup.isEmpty())
        settings.endGroup();

    if(!group.isEmpty())
        settings.endGroup();
}

QVariant SettingsManager::getValue(QString name, QString group, QString subGroup)
{
    if(!group.isEmpty())
        settings.beginGroup(group);

    if(!subGroup.isEmpty())
        settings.beginGroup(subGroup);

    QVariant value = settings.value(name);

    if(!subGroup.isEmpty())
        settings.endGroup();

    if(!group.isEmpty())
        settings.endGroup();

    return value;
}
