import { GamePersistence } from '../../persistence';
import type { ServerInfo_PlayerProperties } from 'generated/proto/serverinfo_playerproperties_pb';
import type { GameEventMeta } from 'types';

export function joinGame(data: { playerProperties: ServerInfo_PlayerProperties }, meta: GameEventMeta): void {
  GamePersistence.playerJoined(meta.gameId, data.playerProperties);
}
