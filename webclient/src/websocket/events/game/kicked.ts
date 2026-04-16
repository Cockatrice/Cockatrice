import { Enriched } from '@app/types';
import { WebClient } from '../../WebClient';

export function kicked(_data: {}, meta: Enriched.GameEventMeta): void {
  WebClient.instance.response.game.kicked(meta.gameId);
}
