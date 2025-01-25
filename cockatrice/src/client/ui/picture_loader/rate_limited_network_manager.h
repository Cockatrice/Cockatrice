#ifndef COCKATRICE_RATE_LIMITED_NETWORK_MANAGER_H
#define COCKATRICE_RATE_LIMITED_NETWORK_MANAGER_H

#include <QNetworkAccessManager>
#include <QObject>

class QTimer;

class RateLimitedNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    RateLimitedNetworkManager(const int _maxRequestsPerSecond, QObject *parent = nullptr);
    QNetworkReply *getRateLimited(const QNetworkRequest &request);

private slots:
    void onIntervalTimerTimeout();

private:
    int maxRequestsPerSecond;
    int currentRequestsCount;
    QTimer *intervalTimer;
};

#endif // COCKATRICE_RATE_LIMITED_NETWORK_MANAGER_H
