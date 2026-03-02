/**
 * @file handle_public_servers.h
 * @ingroup Server
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_HANDLE_PUBLIC_SERVERS_H
#define COCKATRICE_HANDLE_PUBLIC_SERVERS_H

#include "user/user_info_connection.h"

class QNetworkReply;
class QNetworkAccessManager;

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
    void sigPublicServersDownloadedUnsuccessfully(int);

public:
    explicit HandlePublicServers(QObject *parent = nullptr);
    ~HandlePublicServers() override = default;

public slots:
    void downloadPublicServers();

private slots:
    void actFinishParsingDownloadedData();

private:
    void updateServerINISettings(QMap<QString, QVariant>);

    QStringList publicServersToRemove;
    QMap<QString, std::pair<QString, UserConnection_Information>> savedHostList;
    QNetworkAccessManager *nam;
    QNetworkReply *reply;
};

#endif // COCKATRICE_HANDLE_PUBLIC_SERVERS_H
