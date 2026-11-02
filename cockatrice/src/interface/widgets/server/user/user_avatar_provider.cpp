#include "user_avatar_provider.h"

#include "libcockatrice/protocol/pb/response.pb.h"
#include "libcockatrice/protocol/pb/serverinfo_user.pb.h"
#include "libcockatrice/protocol/pb/session_commands.pb.h"

#include <QtGlobal>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/response_get_user_info.pb.h>
#include <libcockatrice/protocol/pending_command.h>
#include <string>
#include <sys/types.h>

UserAvatarProvider::UserAvatarProvider(AbstractClient *client, QObject *parent) : QObject(parent), client(client)
{
    retryClock.start();
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
    if (const qint64 *deadline = retryAfter.object(userName)) {
        if (retryClock.elapsed() < *deadline) {
            return;
        }
        retryAfter.remove(userName);
    }

    pending.insert(userName);

    Command_GetUserInfo cmd;
    cmd.set_user_name(userName.toStdString());

    PendingCommand *pend = client->prepareSessionCommand(cmd);

    connect(pend, &PendingCommand::finished, this, [this, userName](const Response &r) {
        pending.remove(userName);

        // A transport or server error is not evidence that the user has no
        // avatar. Leave it uncached so a later request can try again.
        if (r.response_code() != Response::RespOk || !r.HasExtension(Response_GetUserInfo::ext)) {
            retryAfter.insert(userName, new qint64(retryClock.elapsed() + RetryDelayMs));
            return;
        }
        const auto &response = r.GetExtension(Response_GetUserInfo::ext);
        if (!response.has_user_info()) {
            retryAfter.insert(userName, new qint64(retryClock.elapsed() + RetryDelayMs));
            return;
        }
        retryAfter.remove(userName);
        const auto &user = response.user_info();
        const std::string &bmp = user.avatar_bmp();

        QPixmap avatar;
        if (!bmp.empty() &&
            !avatar.loadFromData(reinterpret_cast<const uchar *>(bmp.data()), static_cast<uint>(bmp.size()))) {
            retryAfter.insert(userName, new qint64(retryClock.elapsed() + RetryDelayMs));
            return;
        }
        // An empty avatar in a valid response is a real negative result.
        avatarCache.insert(userName, avatar);

        emit avatarUpdated(userName);
    });

    client->sendCommand(pend);
}
