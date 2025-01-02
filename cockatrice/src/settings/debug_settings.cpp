#include "debug_settings.h"

DebugSettings::DebugSettings(QString settingPath, QObject *parent) : SettingsManager(settingPath + "debug.ini", parent)
{
}

bool DebugSettings::getShowCardId()
{
    return getValue("show", "cardId").toBool();
}
