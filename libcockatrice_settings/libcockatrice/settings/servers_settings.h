/**
 * @file servers_settings.h
 * @ingroup NetworkSettings
 * @brief TODO: Document this.
 */

#ifndef SERVERSSETTINGS_H
#define SERVERSSETTINGS_H

#include "settings_manager.h"

#include <QLoggingCategory>
#include <QObject>
#define SERVERSETTINGS_DEFAULT_HOST "server.cockatrice.us"
#define SERVERSETTINGS_DEFAULT_PORT "4748"

inline Q_LOGGING_CATEGORY(ServersSettingsLog, "servers_settings");

class ServersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    int getPreviousHostLogin();
    int getPrevioushostindex(const QString &);
    QStringList getPreviousHostList();
    QString getPrevioushostName();
    QString getHostname(QString defaultHost = SERVERSETTINGS_DEFAULT_HOST);
    QString getPort(QString defaultPort = SERVERSETTINGS_DEFAULT_PORT);
    QString getPlayerName(QString defaultName = "");
    QString getFPHostname(QString defaultHost = SERVERSETTINGS_DEFAULT_HOST);
    QString getFPPort(QString defaultPort = SERVERSETTINGS_DEFAULT_PORT);
    QString getFPPlayerName(QString defaultName = "");
    QString getPassword();
    QString getSaveName(QString defaultname = "");
    QString getSite(QString defaultName = "");
    bool getSavePassword();
    int getAutoConnect();

    void setPreviousHostLogin(int previous);
    void setPrevioushostName(const QString &);
    void setPreviousHostList(QStringList list);
    void setAutoConnect(int autoconnect);
    void setSite(QString site);
    void setFPHostName(QString hostname);
    void setFPPort(QString port);
    void setFPPlayerName(QString playerName);
    void addNewServer(const QString &saveName,
                      const QString &serv,
                      const QString &port,
                      const QString &username,
                      const QString &password,
                      bool savePassword,
                      const QString &site = QString());
    void removeServer(QString servAddr);
    bool updateExistingServer(QString saveName,
                              QString serv,
                              QString port,
                              QString username,
                              QString password,
                              bool savePassword,
                              QString site = QString());

    bool updateExistingServerWithoutLoss(QString saveName,
                                         QString serv = QString(),
                                         QString port = QString(),
                                         QString site = QString());
    void setClearDebugLogStatus(bool abIsChecked);
    bool getClearDebugLogStatus(bool abDefaultValue);

private:
    explicit ServersSettings(const QString &settingPath, QObject *parent = nullptr);
    ServersSettings(const ServersSettings & /*other*/);
};

#endif // SERVERSSETTINGS_H
