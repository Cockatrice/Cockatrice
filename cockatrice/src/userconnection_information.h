#ifndef USERCONNECTION_INFORMATION_H
#define USERCONNECTION_INFORMATION_H

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

class UserConnection_Information
{
private:
    QString saveName;
    QString server;
    QString port;
    QString username;
    QString password;
    bool savePassword;
    bool isCustom;

public:
    UserConnection_Information();
    UserConnection_Information(QString, QString, QString, QString, QString, bool, bool _isCustom = false);
    QString getSaveName()
    {
        return saveName;
    }
    QString getServer()
    {
        return server;
    }
    QString getPort()
    {
        return port;
    }
    QString getUsername()
    {
        return username;
    }
    QString getPassword()
    {
        return password;
    }
    bool getSavePassword()
    {
        return savePassword;
    }
    bool isCustomServer()
    {
        return isCustom;
    }
    QMap<QString, UserConnection_Information> getServerInfo();
    QStringList getServerInfo(const QString &find);
};
#endif
