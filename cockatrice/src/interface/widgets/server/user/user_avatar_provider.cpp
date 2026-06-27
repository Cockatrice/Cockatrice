#include "user_avatar_provider.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response_get_user_info.pb.h>
#include <libcockatrice/protocol/pending_command.h>

UserAvatarProvider::UserAvatarProvider(AbstractClient *client, QObject *parent) : QObject(parent), client(client)
{
}

const QMap<QString, QPixmap> &UserAvatarProvider::cache() const
{
    return avatarCache;
}

void UserAvatarProvider::requestAvatar(const QString &userName)
{
    if (avatarCache.contains(userName) || pending.contains(userName)) {
        return;
    }

    pending.insert(userName);

    Command_GetUserInfo cmd;
    cmd.set_user_name(userName.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);

    connect(pend, &PendingCommand::finished, this, [this, userName](const Response &r) {
        pending.remove(userName);

        const auto &response = r.GetExtension(Response_GetUserInfo::ext);
        const auto &user = response.user_info();
        const std::string &bmp = user.avatar_bmp();

        QPixmap avatar;
        if (!bmp.empty() &&
            avatar.loadFromData(reinterpret_cast<const uchar *>(bmp.data()), static_cast<uint>(bmp.size()))) {
            avatarCache.insert(userName, avatar);
        } else {
            avatarCache.insert(userName, QPixmap());
        }

        emit avatarUpdated(userName);
    });

    client->sendCommand(pend);
}