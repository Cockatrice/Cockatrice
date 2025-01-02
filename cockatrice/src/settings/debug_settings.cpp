#include "debug_settings.h"

#include <QtCore/QFile>

DebugSettings::DebugSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "debug.ini", parent)
{
    // force debug.ini to be created if it doesn't exist yet
    if (!QFile(settingPath + "debug.ini").exists()) {
        setValue(false, "showCardId", "debug");
    }
}

bool DebugSettings::getShowCardId()
{
    return getValue("showCardId", "debug").toBool();
}
