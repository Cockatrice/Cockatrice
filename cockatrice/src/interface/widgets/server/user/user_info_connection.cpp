#include "user_info_connection.h"

#include "../../../../client/settings/cache_settings.h"

#include <QList>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QVariant>
#include <libcockatrice/settings/servers_settings.h>
#include <utility>

UserConnection_Information::UserConnection_Information() = default;

UserConnection_Information::UserConnection_Information(QString _saveName,
                                                       QString _serverName,
                                                       QString _portNum,
                                                       QString _userName,
                                                       QString _pass,
                                                       bool _savePass,
                                                       QString _site)
    : saveName(std::move(_saveName)), server(std::move(_serverName)), port(std::move(_portNum)),
      username(std::move(_userName)), password(std::move(_pass)), savePassword(_savePass), site(std::move(_site))
{
}

QMap<QString, std::pair<QString, UserConnection_Information>> UserConnection_Information::getServerInfo()
{
    QMap<QString, std::pair<QString, UserConnection_Information>> serverList;

    ServersSettings &servers = SettingsCache::instance().servers();

    for (const auto &entry : servers.getServers()) {
        UserConnection_Information userInfo(entry.saveName, entry.host, entry.port, entry.username, entry.password,
                                            entry.savePassword, entry.site);
        serverList.insert(entry.saveName, std::make_pair(entry.host, userInfo));
    }

    return serverList;
}

QStringList UserConnection_Information::getServerInfo(const QString &find)
{
    QStringList _server;

    ServersSettings &servers = SettingsCache::instance().servers();

    for (const auto &entry : servers.getServers()) {
        if (find != entry.saveName) {
            continue;
        }

        _server.append(entry.saveName);
        _server.append(entry.host);
        _server.append(entry.port);
        _server.append(entry.username);
        _server.append(entry.password);
        _server.append(entry.savePassword ? "1" : "0");
        _server.append(entry.site);
        break;
    }

    if (_server.empty()) {
        qCWarning(UserInfoConnectionLog) << "There was a problem!";
    }

    return _server;
}
