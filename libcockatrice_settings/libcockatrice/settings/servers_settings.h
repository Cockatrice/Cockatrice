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
#include <optional>
#define SERVERSETTINGS_DEFAULT_HOST "server.cockatrice.us"
#define SERVERSETTINGS_DEFAULT_PORT "4748"

inline Q_LOGGING_CATEGORY(ServersSettingsLog, "servers_settings");

/**
 * @brief Saved credentials for a server identified by hostname+port.
 *
 * @c password is empty when the entry's @c savePassword flag is false, in
 * which case the caller should prompt the user for the password.
 */
struct SavedServerCreds
{
    QString playerName;
    QString password;
};

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

    /**
     * @brief Look up saved credentials by hostname+port.
     *
     * Used by the URL-driven join flow to authenticate against a server
     * without requiring credentials in the URL itself.  Returns @c nullopt
     * when no saved server matches.  When the matched entry has
     * @c savePassword == false, the returned creds have an empty @c password
     * — the caller is expected to prompt the user.
     *
     * Hostname matching is case-insensitive.
     */
    [[nodiscard]] std::optional<SavedServerCreds> findSavedCredsByHostPort(const QString &host, quint16 port) const;

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
