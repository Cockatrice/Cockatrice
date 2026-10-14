#include "debug_settings.h"

#include "libcockatrice/settings/settings_manager.h"

#include <QFile>
#include <QVariant>

class QObject;

DebugSettings::DebugSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "debug.ini", "debug", QString(), parent)
{
    // Create the default debug.ini if it doesn't exist yet
    if (!QFile(settingPath + "debug.ini").exists()) {
        QFile::copy(":/resources/config/debug.ini", settingPath + "debug.ini");
    }
}

bool DebugSettings::getShowCardId() const
{
    return getValue("showCardId").toBool();
}

bool DebugSettings::getLocalGameOnStartup() const
{
    return getValue("onStartup", "localgame").toBool();
}

int DebugSettings::getLocalGamePlayerCount() const
{
    return getValue("playerCount", "localgame").toInt();
}

QString DebugSettings::getDeckPathForPlayer(const QString &playerName) const
{
    return getValue(playerName, "localgame", "deck").toString();
}

DebugSettings::CrashReportingMode DebugSettings::getCrashReportingMode() const
{
    const int raw = getValue("crashReportingMode", QString(), QString(), 0).toInt();
    switch (raw) {
        case 1:
            return CrashReportingMode::AlwaysSend;
        case 2:
            return CrashReportingMode::NeverSend;
        case 0:
        default:
            // Clamp corrupt ini values back to Ask; the QSettings key string is unchanged (no migration).
            return CrashReportingMode::Ask;
    }
}

void DebugSettings::setCrashReportingMode(CrashReportingMode _crashReportingMode)
{
    setValue(static_cast<int>(_crashReportingMode), "crashReportingMode");
    emit crashReportingModeChanged(_crashReportingMode);
}
