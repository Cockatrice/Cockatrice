#include "handle_public_servers.h"
#include "settingscache.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#define PUBLIC_SERVERS_JSON "https://cockatrice.github.io/public-servers.json"

HandlePublicServers::HandlePublicServers(QObject *parent)
    : QObject(parent), nam(new QNetworkAccessManager(this)), reply(nullptr)
{
}

void HandlePublicServers::downloadPublicServers()
{
    QUrl url(QString(PUBLIC_SERVERS_JSON));
    reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(actFinishParsingDownloadedData()));
}

void HandlePublicServers::actFinishParsingDownloadedData()
{
    reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError) {
        // Get current saved hosts
        UserConnection_Information uci;
        savedHostList = uci.getServerInfo();

        // Downloaded data from GitHub
        QJsonParseError parseError{};
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            QVariantMap jsonMap = jsonResponse.toVariant().toMap();
            updateServerINISettings(jsonMap);
        } else {
            qDebug() << "[PUBLIC SERVER HANDLER]"
                     << "JSON Parsing Error:" << parseError.errorString();
            emit sigPublicServersDownloadedUnsuccessfully(errorCode);
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

        QString serverAddress = serverMap["host"].toString();

        if (serverMap["isInactive"].toBool()) {
            publicServersToRemove.append(serverAddress);
            continue;
        }

        QString serverName = serverMap["name"].toString();
        QString serverPort = serverMap["port"].toString();
        QString serverSite = serverMap["site"].toString();

        if (serverMap.contains("websocketPort")) {
            serverPort = serverMap["websocketPort"].toString();
        }

        bool serverFound = false;
        for (const auto &iter : savedHostList) {
            // If the URL/IP matches
            if (iter.second.getServer() == serverAddress) {
                serverFound = true;
                break;
            }
        }

        if (serverFound) {
            settingsCache->servers().updateExistingServerWithoutLoss(serverName, serverAddress, serverPort, serverSite);
        } else {
            settingsCache->servers().addNewServer(serverName, serverAddress, serverPort, "", "", false);
        }
    }

    // If a server was removed from the public list,
    // we will delete it from the local system.
    // Will not delete "unofficial" servers
    for (const auto &pair : savedHostList) {
        QString serverAddr = pair.first;

        if (publicServersToRemove.indexOf(serverAddr) != -1) {
            settingsCache->servers().removeServer(serverAddr);
        }
    }

    emit sigPublicServersDownloadedSuccessfully();
}
