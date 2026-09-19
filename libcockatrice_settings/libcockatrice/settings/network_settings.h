/**
 * @file network_settings.h
 * @ingroup NetworkSettings
 */
//! \todo Document this file.

#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

#include "settings_manager.h"

class NetworkSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] QString getClientID() const;
    void setClientID(const QString &_clientID);
    [[nodiscard]] QString getClientVersion() const;
    void setClientVersion(const QString &_clientVersion);
    [[nodiscard]] int getKeepAlive() const;
    void setKeepAlive(int _keepAlive);
    [[nodiscard]] int getTimeOut() const;
    void setTimeOut(int _timeOut);
    [[nodiscard]] QString getKnownMissingFeatures() const;
    void setKnownMissingFeatures(const QString &_knownMissingFeatures);

public:
    explicit NetworkSettings(const QString &settingPath, QObject *parent = nullptr);

private:
    NetworkSettings(const NetworkSettings & /*other*/);
};

#endif // NETWORK_SETTINGS_H
