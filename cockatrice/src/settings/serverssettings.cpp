#include "serverssettings.h"
#include <QDebug>

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

void ServersSettings::setPrevioushostName(const QString &name)
{
    setValue(name, "previoushostName", "server");
}

QString ServersSettings::getSaveName(QString defaultname)
{
    int index = getPrevioushostindex(getPrevioushostName());
    QVariant saveName = getValue(QString("saveName%1").arg(index), "server", "server_details");
    return saveName == QVariant() ? defaultname : saveName.toString();
}

QString ServersSettings::getPrevioushostName()
{
    return getValue("previoushostName", "server").toString();
}

int ServersSettings::getPrevioushostindex(const QString &saveName)
{
    int size = getValue("totalServers", "server", "server_details").toInt() + 1;

    for (int i = 0; i < size; i++)
        if (saveName == getValue(QString("saveName%1").arg(i), "server", "server_details").toString())
            return i;

    return -1;
}

void ServersSettings::setHostName(QString hostname)
{
    setValue(hostname, "hostname", "server");
}

QString ServersSettings::getHostname(QString defaultHost)
{
    int index = getPrevioushostindex(getPrevioushostName());
    QVariant hostname = getValue(QString("server%1").arg(index), "server", "server_details");
    return hostname == QVariant() ? defaultHost : hostname.toString();
}

void ServersSettings::setPort(QString port)
{
    setValue(port, "port", "server");
}

QString ServersSettings::getPort(QString defaultPort)
{
    int index = getPrevioushostindex(getPrevioushostName());
    QVariant port = getValue(QString("port%1").arg(index), "server", "server_details");
    qDebug() << "getPort() index = " << index << " port.val = " << port.toString();
    return port == QVariant() ? defaultPort : port.toString();
}

void ServersSettings::setPlayerName(QString playerName)
{
    setValue(playerName, "playername", "server");
}

QString ServersSettings::getPlayerName(QString defaultName)
{
    int index = getPrevioushostindex(getPrevioushostName());
    QVariant name = getValue(QString("username%1").arg(index), "server", "server_details");
    qDebug() << "getPlayerName() index = " << index << " name.val = " << name.toString();
    return name == QVariant() ? defaultName : name.toString();
}

QString ServersSettings::getPassword()
{
    int index = getPrevioushostindex(getPrevioushostName());

    if (getSavePassword())
        return getValue(QString("password%1").arg(index), "server", "server_details").toString();

    return QString();
}

bool ServersSettings::getSavePassword()
{
    int index = getPrevioushostindex(getPrevioushostName());
    bool save = getValue(QString("savePassword%1").arg(index), "server", "server_details").toBool();
    return save;
}

void ServersSettings::setPassword(QString password)
{
    setValue(password, "password", "server");
}

void ServersSettings::setSavePassword(int save)
{
    setValue(save, "save_password", "server");
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

void ServersSettings::setFPHostName(QString hostname)
{
    setValue(hostname, "fphostname", "server");
}

QString ServersSettings::getFPHostname(QString defaultHost)
{
    QVariant hostname = getValue("fphostname", "server");
    return hostname == QVariant() ? defaultHost : hostname.toString();
}

void ServersSettings::setFPPort(QString port)
{
    setValue(port, "fpport", "server");
}

QString ServersSettings::getFPPort(QString defaultPort)
{
    QVariant port = getValue("fpport", "server");
    return port == QVariant() ? defaultPort : port.toString();
}

void ServersSettings::setFPPlayerName(QString playerName)
{
    setValue(playerName, "fpplayername", "server");
}

QString ServersSettings::getFPPlayerName(QString defaultName)
{
    QVariant name = getValue("fpplayername", "server");
    return name == QVariant() ? defaultName : name.toString();
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

void ServersSettings::addNewServer(QString saveName, QString serv, QString port, QString username, QString password, bool savePassword)
{
    if (updateExistingServer(saveName, serv, port, username, password, savePassword))
        return;

    int index = getValue("totalServers", "server", "server_details").toInt() + 1;

    setValue(saveName, QString("saveName%1").arg(index), "server", "server_details");
    setValue(serv, QString("server%1").arg(index), "server", "server_details");
    setValue(port, QString("port%1").arg(index), "server", "server_details");
    setValue(username, QString("username%1").arg(index), "server", "server_details");
    setValue(savePassword, QString("savePassword%1").arg(index), "server", "server_details");
    setValue(index, "totalServers", "server", "server_details");
    setValue(password, QString("password%1").arg(index), "server", "server_details");
    
}

bool ServersSettings::updateExistingServer(QString saveName, QString serv, QString port, QString username, QString password, bool savePassword)
{
    int size = getValue("totalServers", "server", "server_details").toInt() + 1;

    for (int i = 0; i < size; i++)
    {
        if (saveName == getValue(QString("saveName%1").arg(i), "server", "server_details").toString())
        {
            setValue(serv, QString("server%1").arg(i), "server", "server_details");
            setValue(port, QString("port%1").arg(i), "server", "server_details");
            setValue(username, QString("username%1").arg(i), "server", "server_details");
            setValue(savePassword, QString("savePassword%1").arg(i), "server", "server_details");
            setValue(password, QString("password%1").arg(i), "server", "server_details");
            return true;
        }
    }
    return false;
}
