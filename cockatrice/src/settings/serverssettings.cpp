#include "serverssettings.h"

#include <QDebug>
#include <utility>

ServersSettings::ServersSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "servers.ini", parent)
{
}

int ServersSettings::end()
{
    return getValue("totalServers", "server", "server_details").toInt() + start();
}

void ServersSettings::setPreviousHostName(const QString &name)
{
    setValue(name, "previoushostName", "server");
}

QString ServersSettings::getPreviousHostName(const QString &defaultHostName)
{
    const QVariant value = getValue("previoushostName", "server");
    return value == QVariant() ? defaultHostName : value.toString();
}

int ServersSettings::getIndex(const QString &saveName)
{
    for (int i = start(); i < end(); ++i) {
        if (saveName == getSaveName(i))
            return i;
    }

    return -1; // return -1 if not found
}

void ServersSettings::setSaveName(int index, const QString &saveName)
{
    setValue(saveName, QString("saveName%1").arg(index), "server", "server_details");
}

QString ServersSettings::getSaveName(int index)
{
    return getValue(QString("saveName%1").arg(index), "server", "server_details").toString();
}

void ServersSettings::setHostName(int index, const QString &host)
{
    setValue(host, QString("server%1").arg(index), "server", "server_details");
}

QString ServersSettings::getHostName(int index, const QString &defaultHost)
{
    QVariant hostname = getValue(QString("server%1").arg(index), "server", "server_details");
    return hostname == QVariant() ? defaultHost : hostname.toString();
}

void ServersSettings::setPort(int index, unsigned int port)
{
    setValue(port, QString("port%1").arg(index), "server", "server_details");
}

unsigned int ServersSettings::getPort(int index, unsigned int defaultPort)
{
    QVariant port = getValue(QString("port%1").arg(index), "server", "server_details");
    return port == QVariant() ? defaultPort : port.toUInt();
}

void ServersSettings::setPlayerName(int index, const QString &playerName)
{
    setValue(playerName, QString("username%1").arg(index), "server", "server_details");
}

QString ServersSettings::getPlayerName(int index, const QString &defaultName)
{
    QVariant name = getValue(QString("username%1").arg(index), "server", "server_details");
    return name == QVariant() ? defaultName : name.toString();
}

void ServersSettings::setPassword(int index, const QString &password)
{
    setValue(password, QString("password%1").arg(index), "server", "server_details");
}

QString ServersSettings::getPassword(int index)
{
    return getValue(QString("password%1").arg(index), "server", "server_details").toString();
}

void ServersSettings::setSecurePassword(int index, const QString &securePassword)
{
    setValue(securePassword, QString("securePassword%1").arg(index), "server", "server_details");
}

QString ServersSettings::getSecurePassword(int index)
{
    return getValue(QString("securePassword%1").arg(index), "server", "server_details").toString();
}

void ServersSettings::setSavePassword(int index, bool savePassword)
{
    setValue(savePassword, QString("savePassword%1").arg(index), "server", "server_details");
}

bool ServersSettings::getSavePassword(int index)
{
    return getValue(QString("savePassword%1").arg(index), "server", "server_details").toBool();
}

void ServersSettings::setSite(int index, const QString &site)
{
    setValue(site, QString("site%1").arg(index), "server", "server_details");
}

QString ServersSettings::getSite(int index, const QString &defaultSite)
{
    QVariant hostname = getValue(QString("site%1").arg(index), "server", "server_details");
    return hostname == QVariant() ? defaultSite : hostname.toString();
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

void ServersSettings::setClearDebugLogStatus(bool abIsChecked)
{
    setValue(abIsChecked, "save_debug_log", "server");
}

bool ServersSettings::getClearDebugLogStatus(bool abDefaultValue)
{
    QVariant cbFlushLog = getValue("save_debug_log", "server");
    return cbFlushLog == QVariant() ? abDefaultValue : cbFlushLog.toBool();
}

int ServersSettings::addNewServer(const QString &saveName, const QString &hostName, unsigned int port)
{
    int index = getIndex(saveName); // check if already present
    if (index == -1) {
        // NOTE: totalServers starts off as 0, incrementing here means the first index will be 1
        index = getValue("totalServers", "server", "server_details").toInt() + 1;
        setValue(index, "totalServers", "server", "server_details");
    }

    QString indexString = QString::number(index);
    setValue(saveName, "saveName" + indexString, "server", "server_details");
    setValue(hostName, "server" + indexString, "server", "server_details");
    setValue(port, "port" + indexString, "server", "server_details");

    return index;
}

int ServersSettings::removeHostName(const QString &hostName)
{
    static const QStringList allKeys{"saveName",       "server",       "port",     "password",
                                     "securePassword", "savePassword", "username", "site"};

    int found = 0;
    for (int i = start(); i < end(); ++i) {
        if (found != 0) {
            // move all other entries after the found entry forwards, overwriting the previous ones
            int previous = i - found;
            QString previousString = QString::number(previous);
            QString currentString = QString::number(i);
            for (const auto &key : allKeys) {
                QString previousKey = key + previousString;
                deleteValue(previousKey, "server", "server_details"); // make sure all entries are removed of target
                QString currentKey = key + currentString;
                QVariant value = getValue(currentKey, "server", "server_details");
                if (value != QVariant()) { // if the source key is in use
                    setValue(value, previousKey, "server", "server_details");
                }
            }
        }
        // find entry to overwrite
        if (hostName == getHostName(i)) {
            ++found;
        }
    }

    // if we have deleted an entry, adjust the total
    if (found != 0) {
        int oldEnd = end();
        int newSize = getValue("totalServers", "server", "server_details").toInt() - found;
        setValue(newSize, "totalServers", "server", "server_details"); // this changes end()

        // delete all values that are outside of our new iterating range
        for (int i = end(); i < oldEnd; ++i) {
            QString previousString = QString::number(i);
            for (const auto &key : allKeys) {
                QString previousKey = key + previousString;
                deleteValue(previousKey, "server", "server_details"); // make sure all entries are removed of target
            }
        }
    }
    return found;
}

// deprecated, here to support older configs
void ServersSettings::setFPHostName(const QString &hostname)
{
    setValue(hostname, "fphostname", "server");
}

QString ServersSettings::getFPHostname(const QString &defaultHost)
{
    QVariant hostname = getValue("fphostname", "server");
    return hostname == QVariant() ? defaultHost : hostname.toString();
}

void ServersSettings::setFPPort(const QString &port)
{
    setValue(port, "fpport", "server");
}

QString ServersSettings::getFPPort(const QString &defaultPort)
{
    QVariant port = getValue("fpport", "server");
    return port == QVariant() ? defaultPort : port.toString();
}

void ServersSettings::setFPPlayerName(const QString &playerName)
{
    setValue(playerName, "fpplayername", "server");
}

QString ServersSettings::getFPPlayerName(const QString &defaultName)
{
    QVariant name = getValue("fpplayername", "server");
    return name == QVariant() ? defaultName : name.toString();
}
