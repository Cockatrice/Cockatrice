import type { Enriched } from '@app/types';

export interface GamesState {
  games: { [gameId: number]: Enriched.GameEntry };
}
