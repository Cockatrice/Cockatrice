/**
 * @file debug_settings.h
 * @ingroup CoreSettings
 * @brief TODO: Document this.
 */

#ifndef DEBUG_SETTINGS_H
#define DEBUG_SETTINGS_H
#include "settings_manager.h"

class DebugSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

    explicit DebugSettings(const QString &settingPath, QObject *parent = nullptr);
    DebugSettings(const DebugSettings & /*other*/);

public:
    bool getShowCardId();

    bool getLocalGameOnStartup();
    int getLocalGamePlayerCount();

    QString getDeckPathForPlayer(const QString &playerName);
};

#endif // DEBUG_SETTINGS_H
