#ifndef SERVERSSETTINGS_H
#define SERVERSSETTINGS_H

#include "settingsmanager.h"

#include <QObject>

#define SERVERSETTINGS_DEFAULT_NAME "Rooster Ranges"
#define SERVERSETTINGS_DEFAULT_HOST "server.cockatrice.us"
#define SERVERSETTINGS_DEFAULT_PORT 4748

class ServersSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    inline int start() const
    {
        return 1; // the first entry will have 1 appended to it in the ini, not 0
    }
    int end();
    int getIndex(const QString &saveName);
    QString getPreviousHostName(const QString &defaultHostName = SERVERSETTINGS_DEFAULT_NAME);
    QString getSaveName(int index);
    QString getHostName(int index, const QString &defaultHost = SERVERSETTINGS_DEFAULT_HOST);
    unsigned int getPort(int index, unsigned int defaultPort = SERVERSETTINGS_DEFAULT_PORT);
    QString getPlayerName(int index, const QString &defaultName = "");
    QString getPassword(int index);
    QString getSecurePassword(int index);
    QString getSite(int index, const QString &defaultName = "");
    bool getSavePassword(int index);
    int getAutoConnect();
    bool getClearDebugLogStatus(bool abDefaultValue);
    QString getFPHostname(const QString &defaultHost = SERVERSETTINGS_DEFAULT_HOST);              // deprecated
    QString getFPPort(const QString &defaultPort = QString::number(SERVERSETTINGS_DEFAULT_PORT)); // deprecated
    QString getFPPlayerName(const QString &defaultName = "");                                     // deprecated

    void setPreviousHostName(const QString &name);
    void setAutoConnect(int autoconnect);
    void setSite(int index, const QString &site);
    void setSaveName(int index, const QString &saveName);
    void setHostName(int index, const QString &host);
    void setPort(int index, unsigned int port);
    void setPlayerName(int index, const QString &playerName);
    void setPassword(int index, const QString &password);
    void setSecurePassword(int index, const QString &securePassword);
    void setSavePassword(int index, bool savePassword);
    void setClearDebugLogStatus(bool abIsChecked);
    void setFPHostName(const QString &hostname);     // deprecated
    void setFPPort(const QString &port);             // deprecated
    void setFPPlayerName(const QString &playerName); // deprecated

    int addNewServer(const QString &saveName, const QString &hostName, unsigned int port);
    int removeHostName(const QString &hostName);

private:
    explicit ServersSettings(const QString &settingPath, QObject *parent = nullptr);
    ServersSettings(const ServersSettings & /*other*/);
};

#endif // SERVERSSETTINGS_H
