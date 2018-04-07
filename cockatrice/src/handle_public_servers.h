//
// Created by Zachary Halpern on 4/6/18.
//
#ifndef COCKATRICE_HANDLE_PUBLIC_SERVERS_H
#define COCKATRICE_HANDLE_PUBLIC_SERVERS_H

#include "userconnection_information.h"
#include <QNetworkReply>

/**
 * This class is used to update the servers.ini file and ensure
 * the list of public servers has up-to-date information.
 * Servers that are added manually by users are not modified.
 */
class HandlePublicServers : public QObject
{
    Q_OBJECT
signals:
    void sigPublicServersDownloadedSuccessfully();
    void sigPublicServersDownloadedUnsuccessfully();

public:
    explicit HandlePublicServers(QObject *parent = nullptr);
    ~HandlePublicServers() = default;

public slots:
    void downloadPublicServers();

private slots:
    void actFinishParsingDownloadedData();
    void clearNAM();

private:
    QMap<QString, UserConnection_Information> savedHostList;
    QNetworkAccessManager *nam;
    QNetworkReply *reply;
};

#endif // COCKATRICE_HANDLE_PUBLIC_SERVERS_H
