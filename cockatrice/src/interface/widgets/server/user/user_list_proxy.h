/**
 * @file user_list_proxy.h
 * @ingroup UI
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_USERLISTPROXY_H
#define COCKATRICE_USERLISTPROXY_H

class QString;
class ServerInfo_User;

/**
 * Responsible for providing a bare-bones minimal interface into userlist information,
 * including your current connection to the server as well as buddy/ignore/alluser lists.
 */
class UserListProxy
{
public:
    [[nodiscard]] virtual bool isOwnUserRegistered() const = 0;
    [[nodiscard]] virtual QString getOwnUsername() const = 0;
    [[nodiscard]] virtual bool isUserBuddy(const QString &userName) const = 0;
    [[nodiscard]] virtual bool isUserIgnored(const QString &userName) const = 0;
    [[nodiscard]] virtual const ServerInfo_User *getOnlineUser(const QString &userName) const = 0; // Can return nullptr
};

#endif // COCKATRICE_USERLISTPROXY_H
