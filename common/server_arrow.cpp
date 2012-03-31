#include "server_arrow.h"
#include "server_player.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "pb/serverinfo_arrow.pb.h"

Server_Arrow::Server_Arrow(int _id, Server_Card *_startCard, Server_ArrowTarget *_targetItem, const color &_arrowColor)
        : id(_id),
          startCard(_startCard),
          targetItem(_targetItem),
          arrowColor(_arrowColor)
{
}

void Server_Arrow::getInfo(ServerInfo_Arrow *info)
{
	info->set_id(id);
	info->set_start_player_id(startCard->getZone()->getPlayer()->getPlayerId());
	info->set_start_zone(startCard->getZone()->getName().toStdString());
	info->set_start_card_id(startCard->getId());
	info->mutable_arrow_color()->CopyFrom(arrowColor);

	Server_Card *targetCard = qobject_cast<Server_Card *>(targetItem);
	if (targetCard) {
		info->set_target_player_id(targetCard->getZone()->getPlayer()->getPlayerId());
		info->set_target_zone(targetCard->getZone()->getName().toStdString());
		info->set_target_card_id(targetCard->getId());
	} else
		info->set_target_player_id(static_cast<Server_Player *>(targetItem)->getPlayerId());
}
