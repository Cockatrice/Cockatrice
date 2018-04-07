#include "handle_public_servers.h"
#include "qt-json/json.h"
#include "settingscache.h"
#include <QUrl>

#define PUBLIC_SERVERS_JSON "https://cockatrice.github.io/public-servers.json"

HandlePublicServers::HandlePublicServers(QObject *parent) : QObject(parent), nam(nullptr), reply(nullptr)
{
}

void HandlePublicServers::downloadPublicServers()
{
    QUrl url(QString(PUBLIC_SERVERS_JSON));
    nam = new QNetworkAccessManager(this);
    reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(actFinishParsingDownloadedData()));
}

void HandlePublicServers::actFinishParsingDownloadedData()
{
    reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    // After finished() is called, this object will be deleted
    nam->deleteLater();

    if (errorCode == QNetworkReply::NoError) {
        // Get current saved hosts
        UserConnection_Information uci;
        savedHostList = uci.getServerInfo();

        // Downloaded data from GitHub
        bool jsonSuccessful;
        QString jsonData = QString(reply->readAll());

        auto jsonMap = QtJson::Json::parse(jsonData, jsonSuccessful).toMap();

        if (!jsonSuccessful) {
            qDebug() << "[PUBLIC SERVER HANDLER]"
                     << "JSON Parsing Error";
            emit sigPublicServersDownloadedUnsuccessfully(errorCode);
        } else {
            updateServerINISettings(jsonMap);
        }

    } else {
        qDebug() << "[PUBLIC SERVER HANDLER]"
                 << "Error Downloading Public Servers" << errorCode;
        emit sigPublicServersDownloadedUnsuccessfully(errorCode);
    }

    reply->deleteLater(); // After an emit() occurs, this object will be deleted
}

void HandlePublicServers::updateServerINISettings(QMap<QString, QVariant> jsonMap)
{
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

        bool serverFound = false;
        for (const auto &iter : savedHostList) {
            if (iter.first == serverName) {
                serverFound = true;
                break;
            }
        }

        if (!serverFound) {
            settingsCache->servers().addNewServer(serverName, serverAddress, serverPort, "", "", false);
        } else {
            settingsCache->servers().updateExistingServerWithoutLoss(serverName, serverAddress, serverPort);
        }
    }

    // If a server was removed from the public list,
    // we will delete it from the local system.
    // Will not delete "unofficial" servers
    for (const auto &pair : savedHostList) {
        QString serverAddr = pair.first;
        QString serverName = pair.second.getSaveName();
        bool isCustom = pair.second.isCustomServer();

        if (!isCustom && publicServersToRemove.indexOf(serverAddr) != -1) {
            settingsCache->servers().removeServer(serverName);
        }
    }

    emit sigPublicServersDownloadedSuccessfully();
}
