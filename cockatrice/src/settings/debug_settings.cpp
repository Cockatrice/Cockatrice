#include "debug_settings.h"

DebugSettings::DebugSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "debug.ini", parent)
{
}

bool DebugSettings::getShowCardId()
{
    return getValue("showCardId", "debug").toBool();
}
