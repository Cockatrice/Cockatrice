#ifndef COCKATRICE_ARROW_DATA_H
#define COCKATRICE_ARROW_DATA_H

#include <QColor>
#include <QString>
#include <libcockatrice/protocol/pb/serverinfo_arrow.pb.h>
#include <libcockatrice/utility/color.h>

struct ArrowData
{
    int creatorId = -1;
    bool isLocalCreator = false;
    int id = -1;
    int startPlayerId = -1;
    QString startZone = "";
    int startCardId = -1;
    int targetPlayerId = -1;
    QString targetZone = "";
    int targetCardId = -1;
    QColor color = "";

    static ArrowData fromProto(const ServerInfo_Arrow &arrow, int creatorId, bool isLocalCreator);

    bool isPlayerTargeted() const
    {
        return targetZone.isEmpty();
    }
};

#endif // COCKATRICE_ARROW_DATA_H
