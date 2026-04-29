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
    int getPreviousHostLogin() const;
    int getPrevioushostindex(const QString &) const;
    QStringList getPreviousHostList() const;
    QString getPrevioushostName() const;
    QString getHostname(QString defaultHost = SERVERSETTINGS_DEFAULT_HOST) const;
    QString getPort(QString defaultPort = SERVERSETTINGS_DEFAULT_PORT) const;
    QString getPlayerName(QString defaultName = "") const;
    QString getFPHostname(QString defaultHost = SERVERSETTINGS_DEFAULT_HOST) const;
    QString getFPPort(QString defaultPort = SERVERSETTINGS_DEFAULT_PORT) const;
    QString getFPPlayerName(QString defaultName = "") const;
    QString getPassword();
    QString getSaveName(QString defaultname = "");
    QString getSite(QString defaultName = "");
    bool getSavePassword() const;
    int getAutoConnect() const;

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
    int findServerIndex(const QString &host, const QString &port) const;
    bool hasUsername(const QString &host, const QString &port) const;
    bool hasCredentials(const QString &host, const QString &port) const;
    bool hasLoginData(const QString &host, const QString &port) const;

    bool updateExistingServerWithoutLoss(QString saveName,
                                         QString serv = QString(),
                                         QString port = QString(),
                                         QString site = QString());
    void setClearDebugLogStatus(bool abIsChecked);
    bool getClearDebugLogStatus(bool abDefaultValue) const;

private:
    explicit ServersSettings(const QString &settingPath, QObject *parent = nullptr);
    ServersSettings(const ServersSettings & /*other*/);
};

#endif // SERVERSSETTINGS_H
