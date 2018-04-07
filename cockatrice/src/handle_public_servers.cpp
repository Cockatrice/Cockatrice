//
// Created by Zachary Halpern on 4/6/18.
//

#include "handle_public_servers.h"
#include "qt-json/json.h"
#include "settingscache.h"
#include <QUrl>

#define PUBLIC_SERVERS_JSON "https://cockatrice.github.io/public-servers.json"

HandlePublicServers::HandlePublicServers(QObject *parent) : QObject(parent)
{
}

void HandlePublicServers::downloadPublicServers()
{
    QUrl url(QString(PUBLIC_SERVERS_JSON));
    nam = new QNetworkAccessManager(this);
    reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(actFinishParsingDownloadedData()));
    connect(reply, SIGNAL(finished()), this, SLOT(clearNAM()));
}

void HandlePublicServers::clearNAM()
{
    nam->deleteLater(); // After finished() is called, this object will be deleted
}

void HandlePublicServers::actFinishParsingDownloadedData()
{
    reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError) {
        // Get current saved hosts
        UserConnection_Information uci;
        savedHostList = uci.getServerInfo();

        // List of public servers
        // Will be populated below
        QStringList publicServersAvailable;
        QStringList publicServersToRemove;

        // Downloaded data from GitHub
        QString jsonData = QString(reply->readAll());
        auto jsonMap = QtJson::Json::parse(jsonData).toMap();

        // Servers available
        auto publicServersJSONList = jsonMap["servers"].toList();
        for (const auto &server : publicServersJSONList) {
            // Data inside one server at a time
            // server: [{ ... }, ..., { ... }]
            const auto serverMap = server.toMap();

            QString serverName = serverMap["name"].toString();

            if (serverMap["isInactive"].toBool()) {
                publicServersToRemove.append(serverName);
                continue;
            }

            QString serverAddress = serverMap["host"].toString();
            QString serverPort = serverMap["port"].toString();

            publicServersAvailable.append(serverName);

            if (!savedHostList.contains(serverName)) {
                settingsCache->servers().addNewServer(serverName, serverAddress, serverPort, "", "", false);
            } else {
                settingsCache->servers().updateExistingServerWithoutLoss(serverName, serverAddress, serverPort);
            }
        }

        // If a server was removed from the public list,
        // we will delete it from the local system.
        // Will not delete "unofficial" servers
        for (auto server : savedHostList) {
            QString serverName = server.getSaveName();
            bool isCustom = server.isCustomServer();

            if (!isCustom && publicServersToRemove.indexOf(serverName) != -1) {
                settingsCache->servers().removeServer(serverName);
            }
        }

        emit sigPublicServersDownloadedSuccessfully();
    } else {
        qDebug() << "[CONNECTION DIALOG]"
                 << "Error Downloading Public Servers" << errorCode;
        emit sigPublicServersDownloadedUnsuccessfully();
    }

    reply->deleteLater(); // After an emit() occurs, this object will be deleted
}
