#include "serverssettings.h"

ServersSettings::ServersSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath+"servers.ini", parent)
{
}

void ServersSettings::setPreviousHostLogin(int previous)
{
    setValue(previous, "previoushostlogin", "server");
}

int ServersSettings::getPreviousHostLogin()
{
    QVariant previous = getValue("previoushostlogin", "server");
    return previous == QVariant() ? 1 : previous.toInt();
}

void ServersSettings::setPreviousHostList(QStringList list)
{
    setValue(list, "previoushosts", "server");
}

QStringList ServersSettings::getPreviousHostList()
{
    return getValue("previoushosts", "server").toStringList();
}

void ServersSettings::setPrevioushostindex(int index)
{
    setValue(index, "previoushostindex", "server");
}

int ServersSettings::getPrevioushostindex()
{
    return getValue("previoushostindex", "server").toInt();
}

void ServersSettings::setHostName(QString hostname)
{
    setValue(hostname, "hostname", "server");
}

QString ServersSettings::getHostname(QString defaultHost)
{
    QVariant hostname = getValue("hostname","server");
    return hostname == QVariant() ? defaultHost : hostname.toString();
}

void ServersSettings::setPort(QString port)
{
    setValue(port, "port", "server");
}

QString ServersSettings::getPort(QString defaultPort)
{
    QVariant port = getValue("port","server");
    return port == QVariant() ? defaultPort : port.toString();
}

void ServersSettings::setPlayerName(QString playerName)
{
    setValue(playerName, "playername", "server");
}

QString ServersSettings::getPlayerName(QString defaultName)
{
    QVariant name = getValue("playername", "server");
    return name == QVariant() ? defaultName : name.toString();
}

void ServersSettings::setPassword(QString password)
{
    setValue(password, "password", "server");
}

QString ServersSettings::getPassword()
{
    return getValue("password", "server").toString();
}

void ServersSettings::setSavePassword(int save)
{
    setValue(save, "save_password", "server");
}

int ServersSettings::getSavePassword()
{
    QVariant save = getValue("save_password", "server");
    return save == QVariant() ? 1 : save.toInt();
}

void ServersSettings::setAutoConnect(int autoconnect)
{
    setValue(autoconnect, "auto_connect", "server");
}

int ServersSettings::getAutoConnect()
{
    QVariant autoconnect = getValue("auto_connect", "server");
    return autoconnect == QVariant() ? 0 : autoconnect.toInt();
}
