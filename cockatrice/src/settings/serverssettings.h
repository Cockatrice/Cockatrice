#ifndef SERVERSSETTINGS_H
#define SERVERSSETTINGS_H

#include "settingsmanager.h"
#include <QObject>

class ServersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    int getPreviousHostLogin();
    int getPrevioushostindex(const QString &);
    QStringList getPreviousHostList();
    QString getPrevioushostName();
    QString getHostname(QString defaultHost = "");
    QString getPort(QString defaultPort = "");
    QString getPlayerName(QString defaultName = "");
    QString getFPHostname(QString defaultHost = "");
    QString getFPPort(QString defaultPort = "");
    QString getFPPlayerName(QString defaultName = "");
    QString getPassword();
    QString getSaveName(QString defaultname = "");
    QString getSite(QString defaultName = "");
    bool getSavePassword();
    int getAutoConnect();

    void setPreviousHostLogin(int previous);
    void setPrevioushostName(const QString &);
    void setPreviousHostList(QStringList list);
    void setHostName(QString hostname);
    void setPort(QString port);
    void setPlayerName(QString playerName);
    void setAutoConnect(int autoconnect);
    void setSite(QString site);
    void setFPHostName(QString hostname);
    void setPassword(QString password);
    void setFPPort(QString port);
    void setSavePassword(int save);
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
    explicit ServersSettings(QString settingPath, QObject *parent = nullptr);
    ServersSettings(const ServersSettings & /*other*/);
};

#endif // SERVERSSETTINGS_H
