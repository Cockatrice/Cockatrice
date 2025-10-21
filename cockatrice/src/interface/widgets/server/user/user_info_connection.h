/**
 * @file user_info_connection.h
 * @ingroup Client
 * @brief TODO: Document this.
 */

#ifndef USERCONNECTION_INFORMATION_H
#define USERCONNECTION_INFORMATION_H

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QSettings>
#include <QStandardPaths>

inline Q_LOGGING_CATEGORY(UserInfoConnectionLog, "user_info_connection");

class UserConnection_Information
{
private:
    QString saveName;
    QString server;
    QString port;
    QString username;
    QString password;
    bool savePassword;
    QString site;

public:
    UserConnection_Information();
    UserConnection_Information(QString, QString, QString, QString, QString, bool, QString);
    QString getSaveName() const
    {
        return saveName;
    }
    QString getServer() const
    {
        return server;
    }
    QString getPort() const
    {
        return port;
    }
    QString getUsername() const
    {
        return username;
    }
    QString getPassword() const
    {
        return password;
    }
    bool getSavePassword() const
    {
        return savePassword;
    }
    QString getSite() const
    {
        return site;
    }
    QMap<QString, std::pair<QString, UserConnection_Information>> getServerInfo();
    QStringList getServerInfo(const QString &find);
};
#endif
