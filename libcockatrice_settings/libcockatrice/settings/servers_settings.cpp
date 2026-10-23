#include "servers_settings.h"

#include "libcockatrice/settings/settings_manager.h"

#include <QDebug>
#include <QList>
#include <QMessageLogger>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <optional>
#include <utility>

namespace
{
constexpr auto SERVER_GROUP = "server";
constexpr auto SERVER_DETAILS_GROUP = "server_details";
constexpr auto SERVERS_ARRAY = "servers";

QString legacyKey(const char *field, int index)
{
    return QStringLiteral("%1/%2%3")
        .arg(QString::fromLatin1(SERVER_DETAILS_GROUP), QString::fromLatin1(field))
        .arg(index);
}

void writeServerArray(QSettings &settings, const QList<ServersSettings::ServerEntry> &servers)
{
    settings.beginGroup(SERVER_DETAILS_GROUP);
    settings.beginWriteArray(SERVERS_ARRAY, servers.size());
    for (int i = 0; i < servers.size(); ++i) {
        settings.setArrayIndex(i);
        const ServersSettings::ServerEntry &entry = servers.at(i);
        settings.setValue("saveName", entry.saveName);
        settings.setValue("server", entry.host);
        settings.setValue("port", entry.port);
        settings.setValue("username", entry.username);
        settings.setValue("password", entry.password);
        settings.setValue("savePassword", entry.savePassword);
        settings.setValue("site", entry.site);
    }
    settings.endArray();
    settings.endGroup();
}
} // namespace

ServersSettings::ServersSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "servers.ini", SERVER_GROUP, QString(), parent)
{
    migrateLegacyServers();
}

QList<ServersSettings::ServerEntry> ServersSettings::getServers() const
{
    return loadServers();
}

std::optional<ServersSettings::ServerEntry> ServersSettings::getServer(int index) const
{
    const QList<ServerEntry> servers = loadServers();
    if (index < 0 || index >= servers.size()) {
        return std::nullopt;
    }
    return servers.at(index);
}

void ServersSettings::setPreviousHostLogin(int previous)
{
    setValue(previous, "previousHostLogin");
}

int ServersSettings::getPreviousHostLogin() const
{
    QVariant previous = getValue("previousHostLogin");
    return previous == QVariant() ? 1 : previous.toInt();
}

void ServersSettings::setPreviousHostList(QStringList list)
{
    setValue(list, "previousHosts");
}

QStringList ServersSettings::getPreviousHostList() const
{
    return getValue("previousHosts").toStringList();
}

void ServersSettings::setPrevioushostName(const QString &name)
{
    setValue(name, "previousHostName");
}

QString ServersSettings::getSaveName(QString defaultname)
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    return server ? server->saveName : std::move(defaultname);
}

QString ServersSettings::getSite(QString defaultSite)
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    return server ? server->site : std::move(defaultSite);
}

QString ServersSettings::getPrevioushostName() const
{
    QVariant value = getValue("previousHostName");
    return value == QVariant() ? "Rooster Ranges" : value.toString();
}

int ServersSettings::getPrevioushostindex(const QString &saveName) const
{
    const QList<ServerEntry> servers = loadServers();
    for (int i = 0; i < servers.size(); ++i) {
        if (servers.at(i).saveName == saveName) {
            return i;
        }
    }

    return -1;
}

QString ServersSettings::getHostname(QString defaultHost) const
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    return server ? server->host : std::move(defaultHost);
}

QString ServersSettings::getPort(QString defaultPort) const
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    qCDebug(ServersSettingsLog) << "getPort() found server = " << server.has_value();
    return server ? server->port : std::move(defaultPort);
}

QString ServersSettings::getPlayerName(QString defaultName) const
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    qCDebug(ServersSettingsLog) << "getPlayerName() found server = " << server.has_value();
    return server ? server->username : std::move(defaultName);
}

QString ServersSettings::getPassword()
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    if (!server || !server->savePassword) {
        return QString();
    }

    return server->password;
}

bool ServersSettings::getSavePassword() const
{
    const auto server = getServer(getPrevioushostindex(getPrevioushostName()));
    return server && server->savePassword;
}

void ServersSettings::setAutoConnect(int autoconnect)
{
    setValue(autoconnect, "autoConnect");
}

int ServersSettings::getAutoConnect() const
{
    QVariant autoconnect = getValue("autoConnect");
    return autoconnect == QVariant() ? 0 : autoconnect.toInt();
}

void ServersSettings::setFPHostName(QString hostname)
{
    setValue(hostname, "fpHostName");
}

QString ServersSettings::getFPHostname(QString defaultHost) const
{
    QVariant hostname = getValue("fpHostName");
    return hostname == QVariant() ? std::move(defaultHost) : hostname.toString();
}

void ServersSettings::setFPPort(QString port)
{
    setValue(port, "fpPort");
}

QString ServersSettings::getFPPort(QString defaultPort) const
{
    QVariant port = getValue("fpPort");
    return port == QVariant() ? std::move(defaultPort) : port.toString();
}

void ServersSettings::setFPPlayerName(QString playerName)
{
    setValue(playerName, "fpPlayerName");
}

void ServersSettings::setServerPassword(const QString &saveName, const QString &password)
{
    QList<ServerEntry> servers = loadServers();
    for (ServerEntry &server : servers) {
        if (server.saveName == saveName) {
            server.password = password;
            storeServers(servers);
            return;
        }
    }

    // A credential write that silently no-ops is hard to diagnose from a bug
    // report; surface a mismatched profile name instead.
    qCWarning(ServersSettingsLog) << "setServerPassword() could not find profile:" << saveName
                                  << "- password not saved";
}

QString ServersSettings::getFPPlayerName(QString defaultName) const
{
    QVariant name = getValue("fpPlayerName");
    return name == QVariant() ? std::move(defaultName) : name.toString();
}

void ServersSettings::setClearDebugLogStatus(bool abIsChecked)
{
    setValue(abIsChecked, "saveDebugLog");
}

bool ServersSettings::getClearDebugLogStatus(bool abDefaultValue) const
{
    QVariant cbFlushLog = getValue("saveDebugLog");
    return cbFlushLog == QVariant() ? abDefaultValue : cbFlushLog.toBool();
}

void ServersSettings::addNewServer(const QString &saveName,
                                   const QString &serv,
                                   const QString &port,
                                   const QString &username,
                                   const QString &password,
                                   bool savePassword,
                                   const QString &site)
{
    // Match the exact host-plus-port server the caller is adding, so a link or
    // public-server list entry cannot clobber the port (and credentials) of an
    // unrelated entry that happens to share the same hostname.
    const int existingIndex = findServerIndex(serv, port);
    if (existingIndex >= 0) {
        updateServerFields(existingIndex, saveName, username, password, savePassword, site);
        return;
    }

    QList<ServerEntry> servers = loadServers();
    ServerEntry entry;
    entry.saveName = saveName;
    entry.host = serv;
    entry.port = port;
    entry.username = username;
    entry.password = password;
    entry.savePassword = savePassword;
    entry.site = site;
    servers.append(entry);
    storeServers(servers);
}

void ServersSettings::removeServer(QString servAddr)
{
    QList<ServerEntry> servers = loadServers();
    for (int i = 0; i < servers.size(); ++i) {
        if (servers.at(i).host == servAddr) {
            servers.removeAt(i);
            storeServers(servers);
            return;
        }
    }
}

/**
 * Will only update fields with new values, ignores empty values
 */
bool ServersSettings::updateExistingServerWithoutLoss(QString saveName, QString serv, QString port, QString site)
{
    QList<ServerEntry> servers = loadServers();
    for (ServerEntry &server : servers) {
        if (server.host != serv) {
            continue;
        }

        if (!port.isEmpty()) {
            server.port = port;
        }
        if (!site.isEmpty()) {
            server.site = site;
        }
        server.saveName = saveName;
        storeServers(servers);
        return true;
    }

    return false;
}

bool ServersSettings::updateExistingServer(QString saveName,
                                           QString serv,
                                           QString port,
                                           QString username,
                                           QString password,
                                           bool savePassword,
                                           QString site)
{
    const int index = findServerIndex(serv, port);
    if (index < 0) {
        return false;
    }

    updateServerFields(index, saveName, username, password, savePassword, site);
    return true;
}

void ServersSettings::updateServerFields(int index,
                                         const QString &saveName,
                                         const QString &username,
                                         const QString &password,
                                         bool savePassword,
                                         const QString &site)
{
    QList<ServerEntry> servers = loadServers();
    if (index < 0 || index >= servers.size()) {
        return;
    }

    ServerEntry &server = servers[index];
    if (!username.isEmpty()) {
        server.username = username;
    }
    if (savePassword && !password.isEmpty()) {
        server.password = password;
    } else {
        server.password.clear();
    }
    if (!site.isEmpty()) {
        server.site = site;
    }
    server.savePassword = savePassword;
    server.saveName = saveName;
    storeServers(servers);
}

int ServersSettings::findServerIndex(const QString &host, const QString &port) const
{
    const QList<ServerEntry> servers = loadServers();
    for (int i = 0; i < servers.size(); ++i) {
        if (servers.at(i).host == host && servers.at(i).port == port) {
            return i;
        }
    }

    return -1;
}

int ServersSettings::findHostIndex(const QString &host) const
{
    const QList<ServerEntry> servers = loadServers();
    for (int i = 0; i < servers.size(); ++i) {
        if (servers.at(i).host.compare(host, Qt::CaseInsensitive) == 0) {
            return i;
        }
    }

    return -1;
}

bool ServersSettings::hasUsername(const QString &host, const QString &port) const
{
    const auto server = getServer(findServerIndex(host, port));
    return server && !server->username.isEmpty();
}

bool ServersSettings::hasCredentials(const QString &host, const QString &port) const
{
    const auto server = getServer(findServerIndex(host, port));
    return server && server->savePassword && !server->password.isEmpty();
}

bool ServersSettings::hasLoginData(const QString &host, const QString &port) const
{
    return hasUsername(host, port) && hasCredentials(host, port);
}

QList<ServersSettings::ServerEntry> ServersSettings::loadServers() const
{
    QSettings settings = getSettings();
    QList<ServerEntry> servers;

    settings.beginGroup(SERVER_GROUP);
    settings.beginGroup(SERVER_DETAILS_GROUP);
    const int size = settings.beginReadArray(SERVERS_ARRAY);
    servers.reserve(size);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        ServerEntry entry;
        entry.saveName = settings.value("saveName").toString();
        entry.host = settings.value("server").toString();
        entry.port = settings.value("port").toString();
        entry.username = settings.value("username").toString();
        entry.password = settings.value("password").toString();
        entry.savePassword = settings.value("savePassword").toBool();
        entry.site = settings.value("site").toString();
        servers.append(entry);
    }
    settings.endArray();
    settings.endGroup();
    settings.endGroup();

    return servers;
}

void ServersSettings::storeServers(const QList<ServerEntry> &servers)
{
    batchWrite([&servers](QSettings &settings) {
        settings.beginGroup(SERVER_GROUP);
        writeServerArray(settings, servers);
        settings.endGroup();
    });
}

void ServersSettings::migrateLegacyServers()
{
    {
        QSettings settings = getSettings();
        settings.beginGroup(SERVER_GROUP);
        const bool hasLegacy = settings.contains(QStringLiteral("%1/totalServers").arg(SERVER_DETAILS_GROUP));
        const bool hasArray = settings.contains(QStringLiteral("%1/%2/size").arg(SERVER_DETAILS_GROUP, SERVERS_ARRAY));
        settings.endGroup();

        if (!hasLegacy && !hasArray) {
            return;
        }
    }

    batchWrite([](QSettings &settings) {
        settings.beginGroup(SERVER_GROUP);

        const bool hasArray = settings.contains(QStringLiteral("%1/%2/size").arg(SERVER_DETAILS_GROUP, SERVERS_ARRAY));
        const int lastLegacyIndex =
            settings.value(QStringLiteral("%1/totalServers").arg(SERVER_DETAILS_GROUP), -1).toInt();

        if (!hasArray) {
            QList<ServersSettings::ServerEntry> servers;
            for (int i = 0; i <= lastLegacyIndex; ++i) {
                ServersSettings::ServerEntry entry;
                entry.saveName = settings.value(legacyKey("saveName", i)).toString();
                entry.host = settings.value(legacyKey("server", i)).toString();
                entry.port = settings.value(legacyKey("port", i)).toString();
                entry.username = settings.value(legacyKey("username", i)).toString();
                entry.password = settings.value(legacyKey("password", i)).toString();
                entry.savePassword = settings.value(legacyKey("savePassword", i)).toBool();
                entry.site = settings.value(legacyKey("site", i)).toString();

                // Older builds could leave the unused index 0 behind.
                if (!entry.saveName.isEmpty() || !entry.host.isEmpty()) {
                    servers.append(entry);
                }
            }

            writeServerArray(settings, servers);
            qCDebug(ServersSettingsLog) << "Migrated" << servers.size() << "server profiles to the array format";
        }

        // The array (freshly written or already present) is authoritative now.
        for (int i = 0; i <= lastLegacyIndex; ++i) {
            settings.remove(legacyKey("saveName", i));
            settings.remove(legacyKey("server", i));
            settings.remove(legacyKey("port", i));
            settings.remove(legacyKey("username", i));
            settings.remove(legacyKey("password", i));
            settings.remove(legacyKey("savePassword", i));
            settings.remove(legacyKey("site", i));
        }
        settings.remove(QStringLiteral("%1/totalServers").arg(SERVER_DETAILS_GROUP));
        settings.endGroup();
    });

    QSettings settings = getSettings();
    if (settings.status() != QSettings::NoError) {
        qCWarning(ServersSettingsLog) << "Failed to migrate server profiles:" << settings.status();
    }
}
