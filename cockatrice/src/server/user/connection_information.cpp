#include "userconnection_information.h"

#include "settingscache.h"

#include <QDebug>
#include <utility>

UserConnection_Information::UserConnection_Information() = default;

UserConnection_Information::UserConnection_Information(QString _saveName,
                                                       QString _serverName,
                                                       QString _portNum,
                                                       QString _userName,
                                                       QString _pass,
                                                       bool _savePass,
                                                       QString _site)
    : saveName(std::move(_saveName)), server(std::move(_serverName)), port(std::move(_portNum)),
      username(std::move(_userName)), password(std::move(_pass)), savePassword(_savePass), site(std::move(_site))
{
}

QMap<QString, std::pair<QString, UserConnection_Information>> UserConnection_Information::getServerInfo()
{
    QMap<QString, std::pair<QString, UserConnection_Information>> serverList;

    ServersSettings &servers = SettingsCache::instance().servers();

    int size = servers.getValue("totalServers", "server", "server_details").toInt() + 1;

    for (int i = 0; i < size; i++) {
        QString _saveName = servers.getValue(QString("saveName%1").arg(i), "server", "server_details").toString();
        QString serverName = servers.getValue(QString("server%1").arg(i), "server", "server_details").toString();
        QString portNum = servers.getValue(QString("port%1").arg(i), "server", "server_details").toString();
        QString userName = servers.getValue(QString("username%1").arg(i), "server", "server_details").toString();
        QString pass = servers.getValue(QString("password%1").arg(i), "server", "server_details").toString();
        bool savePass = servers.getValue(QString("savePassword%1").arg(i), "server", "server_details").toBool();
        QString _site = servers.getValue(QString("site%1").arg(i), "server", "server_details").toString();

        UserConnection_Information userInfo(_saveName, serverName, portNum, userName, pass, savePass, _site);
        serverList.insert(_saveName, std::make_pair(serverName, userInfo));
    }

    return serverList;
}

QStringList UserConnection_Information::getServerInfo(const QString &find)
{
    QStringList _server;

    ServersSettings &servers = SettingsCache::instance().servers();

    int size = servers.getValue("totalServers", "server", "server_details").toInt() + 1;
    for (int i = 0; i < size; i++) {
        QString _saveName = servers.getValue(QString("saveName%1").arg(i), "server", "server_details").toString();

        if (find != _saveName)
            continue;

        QString serverName = servers.getValue(QString("server%1").arg(i), "server", "server_details").toString();
        QString portNum = servers.getValue(QString("port%1").arg(i), "server", "server_details").toString();
        QString userName = servers.getValue(QString("username%1").arg(i), "server", "server_details").toString();
        QString pass = servers.getValue(QString("password%1").arg(i), "server", "server_details").toString();
        bool savePass = servers.getValue(QString("savePassword%1").arg(i), "server", "server_details").toBool();
        QString _site = servers.getValue(QString("site%1").arg(i), "server", "server_details").toString();

        _server.append(_saveName);
        _server.append(serverName);
        _server.append(portNum);
        _server.append(userName);
        _server.append(pass);
        _server.append(savePass ? "1" : "0");
        _server.append(_site);
        break;
    }

    if (_server.empty())
        qDebug() << "There was a problem!";

    return _server;
}
