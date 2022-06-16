#ifndef COCKATRICE_HANDLE_PUBLIC_SERVERS_H
#define COCKATRICE_HANDLE_PUBLIC_SERVERS_H

#include "userconnection_information.h"

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
    void sigPublicServersDownloadedUnsuccessfully(int err);

public:
    explicit HandlePublicServers(QObject *parent = nullptr);
    ~HandlePublicServers() override = default;

public slots:
    void downloadPublicServers();

private slots:
    void actFinishParsingDownloadedData();

private:
    void updateServerINISettings(const QMap<QString, QVariant> &jsonMap);

    QNetworkAccessManager *nam;
};

#endif // COCKATRICE_HANDLE_PUBLIC_SERVERS_H
