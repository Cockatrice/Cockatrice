import type { ServerInfo_PlayerProperties } from 'generated/proto/serverinfo_playerproperties_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function playerPropertiesChanged(data: { playerProperties: ServerInfo_PlayerProperties }, meta: GameEventMeta): void {
  GamePersistence.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
