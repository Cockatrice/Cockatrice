import { Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function gameClosed(_data: {}, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.gameClosed(meta.gameId);
}
