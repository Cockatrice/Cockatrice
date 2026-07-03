#include "arrow_data.h"

ArrowData ArrowData::fromProto(const ServerInfo_Arrow &arrow, int creatorId, bool isLocalCreator)
{
    ArrowData data;
    data.creatorId = creatorId;
    data.isLocalCreator = isLocalCreator;
    data.id = arrow.id();
    data.startPlayerId = arrow.start_player_id();
    data.startZone = QString::fromStdString(arrow.start_zone());
    data.startCardId = arrow.start_card_id();
    data.targetPlayerId = arrow.target_player_id();
    data.color = convertColorToQColor(arrow.arrow_color());

    if (arrow.has_target_zone()) {
        data.targetZone = QString::fromStdString(arrow.target_zone());
        data.targetCardId = arrow.target_card_id();
    }

    return data;
}