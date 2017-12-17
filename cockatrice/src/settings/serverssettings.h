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
    bool getSavePassword();
    int getAutoConnect();

    void setPreviousHostLogin(int previous);
    void setPrevioushostName(const QString &);
    void setPreviousHostList(QStringList list);
    void setHostName(QString hostname);
    void setPort(QString port);
    void setPlayerName(QString playerName);
    void setAutoConnect(int autoconnect);
    void setFPHostName(QString hostname);
    void setPassword(QString password);
    void setFPPort(QString port);
    void setSavePassword(int save);
    void setFPPlayerName(QString playerName);
    void addNewServer(QString saveName, QString serv, QString port, QString username, QString password, bool savePassword);
    bool updateExistingServer(QString saveName, QString serv, QString port, QString username, QString password, bool savePassword);
    void setClearDebugLogStatus(bool abIsChecked);
    bool getClearDebugLogStatus(bool abDefaultValue);

signals:

public slots:

private:
    ServersSettings(QString settingPath,QObject *parent = 0);
    ServersSettings( const ServersSettings& /*other*/ );
    ServersSettings( ServersSettings& /*other*/ );
    ServersSettings( volatile const ServersSettings& /*other*/ );
    ServersSettings( volatile ServersSettings& /*other*/ );       
};

#endif // SERVERSSETTINGS_H
