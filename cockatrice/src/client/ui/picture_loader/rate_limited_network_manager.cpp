#include "rate_limited_network_manager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QTimer>

RateLimitedNetworkManager::RateLimitedNetworkManager(const int _maxRequestsPerSecond, QObject *parent)
    : QNetworkAccessManager(parent), maxRequestsPerSecond(_maxRequestsPerSecond), currentRequestsCount(0)
{
    intervalTimer = new QTimer(this);
    connect(intervalTimer, &QTimer::timeout, this, &RateLimitedNetworkManager::onIntervalTimerTimeout);
    intervalTimer->start(1000); // Resets once per second
}

QNetworkReply *RateLimitedNetworkManager::getRateLimited(const QNetworkRequest &request)
{
    if (currentRequestsCount < maxRequestsPerSecond) {
        ++currentRequestsCount;
        qDebug() << "SENDING REQUEST" << request.url();
        return QNetworkAccessManager::get(request);
    }

    // Not on main thread, can sleep
    qDebug() << "SLEEPING ON REQUEST" << request.url();
    QThread::msleep(100);
    return getRateLimited(request);
}

void RateLimitedNetworkManager::onIntervalTimerTimeout()
{

    currentRequestsCount = 0;
}
