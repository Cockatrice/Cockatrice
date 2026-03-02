#include "debug_settings.h"

#include <QtCore/QFile>

DebugSettings::DebugSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "debug.ini", "debug", QString(), parent)
{
    // Create the default debug.ini if it doesn't exist yet
    if (!QFile(settingPath + "debug.ini").exists()) {
        QFile::copy(":/resources/config/debug.ini", settingPath + "debug.ini");
    }
}

bool DebugSettings::getShowCardId()
{
    return getValue("showCardId").toBool();
}

bool DebugSettings::getLocalGameOnStartup()
{
    return getValue("onStartup", "localgame").toBool();
}

int DebugSettings::getLocalGamePlayerCount()
{
    return getValue("playerCount", "localgame").toInt();
}

QString DebugSettings::getDeckPathForPlayer(const QString &playerName)
{
    return getValue(playerName, "localgame", "deck").toString();
}