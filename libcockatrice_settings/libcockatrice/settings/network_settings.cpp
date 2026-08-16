#include "network_settings.h"

NetworkSettings::NetworkSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "network.ini", "network", QString(), parent)
{
}

QString NetworkSettings::getClientID() const
{
    return getValue("clientId", QString(), QString(), "notset").toString();
}

void NetworkSettings::setClientID(const QString &_clientID)
{
    setValue(_clientID, "clientId");
}

QString NetworkSettings::getClientVersion() const
{
    return getValue("clientVersion", QString(), QString(), "notset").toString();
}

void NetworkSettings::setClientVersion(const QString &_clientVersion)
{
    setValue(_clientVersion, "clientVersion");
}

int NetworkSettings::getKeepAlive() const
{
    return getValue("keepAlive", QString(), QString(), 3).toInt();
}

void NetworkSettings::setKeepAlive(int _keepAlive)
{
    setValue(_keepAlive, "keepAlive");
}

int NetworkSettings::getTimeOut() const
{
    return getValue("timeout", QString(), QString(), 5).toInt();
}

void NetworkSettings::setTimeOut(int _timeOut)
{
    setValue(_timeOut, "timeout");
}

QString NetworkSettings::getKnownMissingFeatures() const
{
    return getValue("knownMissingFeatures", QString(), QString(), "").toString();
}

void NetworkSettings::setKnownMissingFeatures(const QString &_knownMissingFeatures)
{
    setValue(_knownMissingFeatures, "knownMissingFeatures");
}
