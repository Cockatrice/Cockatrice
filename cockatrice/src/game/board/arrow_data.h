#ifndef COCKATRICE_ARROW_DATA_H
#define COCKATRICE_ARROW_DATA_H

#include <QColor>
#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_arrow.pb.h>
#include <libcockatrice/utility/color.h>

struct ArrowData
{
    int id;
    int startPlayerId;
    QString startZone;
    int startCardId;
    int targetPlayerId;
    QString targetZone;    // empty = targeting a player
    int targetCardId = -1; // -1 = targeting a player
    QColor color;

    static ArrowData fromProto(const ServerInfo_Arrow &arrow);

    bool isPlayerTargeted() const
    {
        return targetZone.isEmpty();
    }
};

#endif // COCKATRICE_ARROW_DATA_H
