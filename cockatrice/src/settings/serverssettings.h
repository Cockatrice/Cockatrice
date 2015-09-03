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
    QStringList getPreviousHostList();
    int getPrevioushostindex();
    QString getHostname(QString defaultHost = "");
    QString getPort(QString defaultPort = "");
    QString getPlayerName(QString defaultName = "");
    QString getPassword();
    int getSavePassword();
    int getAutoConnect();

    void setPreviousHostLogin(int previous);
    void setPreviousHostList(QStringList list);
    void setPrevioushostindex(int index);
    void setHostName(QString hostname);
    void setPort(QString port);
    void setPlayerName(QString playerName);
    void setPassword(QString password);
    void setSavePassword(int save);
    void setAutoConnect(int autoconnect);
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
