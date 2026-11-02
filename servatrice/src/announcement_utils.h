#ifndef SERVATRICE_ANNOUNCEMENT_UTILS_H
#define SERVATRICE_ANNOUNCEMENT_UTILS_H

#include <QByteArray>
#include <QString>
#include <libcockatrice/protocol/pb/event_notify_user.pb.h>
#include <libcockatrice/utility/string_limits.h>
#include <string>

// The byte limit is checked before conversion: rejecting invalid UTF-8 and
// oversized input avoids truncating or changing the text at the wire boundary.
inline bool normalizeServerAnnouncement(const std::string &input, QString &message)
{
    if (input.size() > static_cast<std::string::size_type>(MAX_TEXT_LENGTH)) {
        return false;
    }
    const QByteArray raw(input.data(), static_cast<int>(input.size()));
    const QString decoded = QString::fromUtf8(raw);
    if (decoded.toUtf8() != raw) {
        return false;
    }
    message = decoded.simplified();
    return !message.isEmpty();
}

inline Event_NotifyUser makeServerAnnouncementEvent(const QString &message)
{
    // This pre-existing event is understood by older clients.
    Event_NotifyUser event;
    event.set_type(Event_NotifyUser::CUSTOM);
    event.set_custom_title("Server announcement");
    event.set_custom_content(message.toStdString());
    return event;
}

#endif
