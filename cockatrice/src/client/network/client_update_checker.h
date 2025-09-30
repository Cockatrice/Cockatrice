/**
 * @file client_update_checker.h
 * @ingroup ClientUpdate
 * @brief TODO: Document this.
 */

#ifndef CLIENT_UPDATE_CHECKER_H
#define CLIENT_UPDATE_CHECKER_H
#include <QObject>

class Release;

/**
 * We use a singleton instance of UpdateChannel, which can cause interference and feedback loops when multiple objects
 * connect to it.
 *
 * This class encapsulates the usage of that UpdateChannel to ensure that the check only happens once per connection and
 * the connection is destroyed after it's been used.
 */
class ClientUpdateChecker : public QObject
{
    Q_OBJECT

    QMetaObject::Connection finishedCheckConnection;
    QMetaObject::Connection errorConnection;

    void actFinishedCheck(bool needToUpdate, bool isCompatible, Release *release);
    void actError(const QString &errorString);

public:
    explicit ClientUpdateChecker(QObject *parent = nullptr);
    /**
     * Actually performs the check, using the currently selected update channel in the settings.
     * Any resulting signals will only be sent once.
     * This method should only be called ONCE per instance.
     */
    void check();

signals:
    /**
     * Forwarded from UpdateChannel::finishedCheck
     */
    void finishedCheck(bool needToUpdate, bool isCompatible, Release *release);

    /**
     * Forwarded from UpdateChannel::error
     */
    void error(const QString &errorString);
};

#endif // CLIENT_UPDATE_CHECKER_H
