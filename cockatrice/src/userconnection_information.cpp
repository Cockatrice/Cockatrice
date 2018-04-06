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
                                                       bool _isCustom)
    : saveName(std::move(_saveName)), server(std::move(_serverName)), port(std::move(_portNum)),
      username(std::move(_userName)), password(std::move(_pass)), savePassword(_savePass), isCustom(_isCustom)
{
}

QMap<QString, UserConnection_Information> UserConnection_Information::getServerInfo()
{
    QMap<QString, UserConnection_Information> serverList;

    int size = settingsCache->servers().getValue("totalServers", "server", "server_details").toInt() + 1;

    for (int i = 0; i < size; i++) {
        QString saveName =
            settingsCache->servers().getValue(QString("saveName%1").arg(i), "server", "server_details").toString();
        QString serverName =
            settingsCache->servers().getValue(QString("server%1").arg(i), "server", "server_details").toString();
        QString portNum =
            settingsCache->servers().getValue(QString("port%1").arg(i), "server", "server_details").toString();
        QString userName =
            settingsCache->servers().getValue(QString("username%1").arg(i), "server", "server_details").toString();
        QString pass =
            settingsCache->servers().getValue(QString("password%1").arg(i), "server", "server_details").toString();
        bool savePass =
            settingsCache->servers().getValue(QString("savePassword%1").arg(i), "server", "server_details").toBool();
        bool isCustom =
            settingsCache->servers().getValue(QString("isCustom%1").arg(i), "server", "server_details").toBool();

        UserConnection_Information userInfo(saveName, serverName, portNum, userName, pass, savePass, isCustom);
        serverList.insert(saveName, userInfo);
    }

    return serverList;
}

QStringList UserConnection_Information::getServerInfo(const QString &find)
{
    QStringList server;

    int size = settingsCache->servers().getValue("totalServers", "server", "server_details").toInt() + 1;
    for (int i = 0; i < size; i++) {
        QString saveName =
            settingsCache->servers().getValue(QString("saveName%1").arg(i), "server", "server_details").toString();

        if (find != saveName)
            continue;

        QString serverName =
            settingsCache->servers().getValue(QString("server%1").arg(i), "server", "server_details").toString();
        QString portNum =
            settingsCache->servers().getValue(QString("port%1").arg(i), "server", "server_details").toString();
        QString userName =
            settingsCache->servers().getValue(QString("username%1").arg(i), "server", "server_details").toString();
        QString pass =
            settingsCache->servers().getValue(QString("password%1").arg(i), "server", "server_details").toString();
        bool savePass =
            settingsCache->servers().getValue(QString("savePassword%1").arg(i), "server", "server_details").toBool();
        bool isCustom =
            settingsCache->servers().getValue(QString("isCustom%1").arg(i), "server", "server_details").toBool();

        server.append(saveName);
        server.append(serverName);
        server.append(portNum);
        server.append(userName);
        server.append(pass);
        server.append(savePass ? "1" : "0");
        server.append(isCustom ? "1" : "0");
        break;
    }

    if (server.empty())
        qDebug() << "There was a problem!";

    return server;
}
