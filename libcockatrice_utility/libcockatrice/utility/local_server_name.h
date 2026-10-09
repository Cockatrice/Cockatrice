#ifndef COCKATRICE_LOCAL_SERVER_NAME_H
#define COCKATRICE_LOCAL_SERVER_NAME_H

#include <QDir>
#include <QString>
#include <QtGlobal>

/**
 * @brief Builds a QLocalServer/QLocalSocket name scoped to the current user.
 *
 * On Linux the default local socket namespace is system-wide, so an unscoped name
 * would let one user's process talk to — or hijack — another user's session.
 * @param prefix application specific socket name prefix
 * @return the scoped socket name
 */
inline QString scopedLocalServerName(const QString &prefix)
{
    QString userName = qEnvironmentVariable("USER");
    if (userName.isEmpty()) {
        userName = qEnvironmentVariable("USERNAME");
    }
    if (userName.isEmpty()) {
        userName = QDir::home().dirName();
    }
    return prefix + QLatin1Char('-') + userName;
}

#endif // COCKATRICE_LOCAL_SERVER_NAME_H
