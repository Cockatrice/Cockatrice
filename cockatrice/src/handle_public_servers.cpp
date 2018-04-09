#include "handle_public_servers.h"
#include "qt-json/json.h"
#include "settingscache.h"
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
        bool jsonSuccessful;
        QString jsonData = QString(reply->readAll());

        auto jsonMap = QtJson::Json::parse(jsonData, jsonSuccessful).toMap();

        if (jsonSuccessful) {
            updateServerINISettings(jsonMap);
        } else {
            qDebug() << "[PUBLIC SERVER HANDLER]"
                     << "JSON Parsing Error";
            emit sigPublicServersDownloadedUnsuccessfully(errorCode);
        }

    } else {
        qDebug() << "[PUBLIC SERVER HANDLER]"
                 << "Error Downloading Public Servers" << errorCode;
        emit sigPublicServersDownloadedUnsuccessfully(errorCode);
    }

    reply->deleteLater(); // After an emit() occurs, this object will be deleted
}

void HandlePublicServers::askToClearServerList()
{
    // Give user option to flush old values if they've never done this before
    QMessageBox messageBox;
    messageBox.setIconPixmap(QPixmap("theme:icons/update"));
    messageBox.setWindowTitle(tr("Spring Cleaning"));
    messageBox.setText(tr("New public server list successfully downloaded.") + "\n" +
                       tr("Do you want to clear out your old server list?") + "\n" +
                       tr("This includes saved usernames and passwords"));
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    auto reply = static_cast<QMessageBox::StandardButton>(messageBox.exec());

    if (reply == QMessageBox::Yes) {
        for (const auto &pair : savedHostList) {
            QString serverName = pair.second.getSaveName();
            settingsCache->servers().removeServer(serverName);
        }
        savedHostList.clear();
        qDebug() << "[PUBLIC SERVER HANDLER]"
                 << "Cleared old saved hosts";
    }
}

void HandlePublicServers::updateServerINISettings(QMap<QString, QVariant> jsonMap)
{
    askToClearServerList();

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

        if (serverFound) {
            settingsCache->servers().updateExistingServerWithoutLoss(serverName, serverAddress, serverPort);
        } else {
            settingsCache->servers().addNewServer(serverName, serverAddress, serverPort, "", "", false);
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
