#include "handle_public_servers.h"

#include "settingscache.h"

#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#define PUBLIC_SERVERS_JSON "https://cockatrice.github.io/public-servers.json"

HandlePublicServers::HandlePublicServers(QObject *parent) : QObject(parent), nam(new QNetworkAccessManager(this))
{
}

void HandlePublicServers::downloadPublicServers()
{
    QUrl url(QString(PUBLIC_SERVERS_JSON));
    QNetworkReply *reply = nam->get(QNetworkRequest(url)); // reply is to be deleted by its signal
    connect(reply, SIGNAL(finished()), this, SLOT(actFinishParsingDownloadedData()));
}

void HandlePublicServers::actFinishParsingDownloadedData()
{
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError) {
        // Downloaded data from GitHub
        QJsonParseError parseError{};
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            QVariantMap jsonMap = jsonResponse.toVariant().toMap();
            updateServerINISettings(jsonMap);
        } else {
            qCritical() << "Error while fetching public servers, json parsing error:" << parseError.errorString();
            emit sigPublicServersDownloadedUnsuccessfully(errorCode);
        }
    } else {
        qCritical() << "Error while fetching public servers, code:" << errorCode;
        emit sigPublicServersDownloadedUnsuccessfully(errorCode);
    }

    reply->deleteLater(); // After an emit() occurs, this object will be deleted
}

void HandlePublicServers::updateServerINISettings(const QMap<QString, QVariant> &jsonMap)
{
    const auto &publicServersJSONList = jsonMap["servers"].toList();
    auto &servers = SettingsCache::instance().servers();

    for (const auto &server : publicServersJSONList) {
        // Data inside one server at a time
        // server: [{ ... }, ..., { ... }]
        const auto &serverMap = server.toMap();

        QString hostName = serverMap["host"].toString();

        if (serverMap["isInactive"].toBool()) {
            servers.removeHostName(hostName);
            continue;
        }

        QString saveName = serverMap["name"].toString();
        unsigned int port;
        if (serverMap.contains("websocketPort")) {
            port = serverMap["websocketPort"].toUInt();
        } else {
            port = serverMap["port"].toUInt();
        }

        int index = servers.addNewServer(saveName, hostName, port);
        if (serverMap.contains("site")) {
            QString site = serverMap["site"].toString();
            servers.setSite(index, site);
        }
    }

    emit sigPublicServersDownloadedSuccessfully();
}
