#include "debug_settings.h"

#include <QtCore/QFile>

DebugSettings::DebugSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "debug.ini", parent)
{
    // force debug.ini to be created if it doesn't exist yet
    if (!QFile(settingPath + "debug.ini").exists()) {
        setValue(false, "showCardId", "debug");

        setValue(false, "onStartup", "localgame");
        setValue(1, "playerCount", "localgame");
        setValue("path/to/deck", "Player 1", "localgame", "deck");
        setValue("path/to/deck", "Player 2", "localgame", "deck");
    }
}

bool DebugSettings::getShowCardId()
{
    return getValue("showCardId", "debug").toBool();
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