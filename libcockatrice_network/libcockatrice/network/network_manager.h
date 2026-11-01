#ifndef COCKATRICE_NETWORK_NETWORK_MANAGER_H
#define COCKATRICE_NETWORK_NETWORK_MANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <mutex>

/**
 * @file network_manager.h
 * @ingroup Network
 * @brief Creates QNetworkAccessManagers with the application-wide baseline.
 *
 * Every manager in the client should start from NetworkManagerFactory::create()
 * so features that apply to all HTTP traffic (the system proxy, a transfer
 * timeout) cannot be forgotten in one of the many call sites. Feature-specific
 * setup such as a disk cache or a manual redirect policy stays with the caller.
 */
namespace NetworkManagerFactory
{
/**
 * @brief Returns a QNetworkAccessManager configured for Cockatrice.
 *
 * The manager is parented to @p parent. The system proxy is enabled once,
 * process-wide, the first time this function is called. Transfers use Qt's
 * default timeout instead of waiting forever.
 */
inline QNetworkAccessManager *create(QObject *parent = nullptr)
{
    static std::once_flag systemProxyFlag;
    std::call_once(systemProxyFlag, [] { QNetworkProxyFactory::setUseSystemConfiguration(true); });

    auto *manager = new QNetworkAccessManager(parent);
    manager->setTransferTimeout();
    return manager;
}
} // namespace NetworkManagerFactory

#endif // COCKATRICE_NETWORK_NETWORK_MANAGER_H
