import { useMemo } from 'react';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Enriched } from '@app/types';

export interface CurrentGame {
  gameId: number | undefined;
  game: Enriched.GameEntry | undefined;
  localPlayer: Enriched.PlayerEntry | undefined;
  isSpectator: boolean;
  isJudge: boolean;
  isHost: boolean;
  isStarted: boolean;
}

export function useCurrentGame(gameId?: number): CurrentGame {
  const activeGameIds = useAppSelector(GameSelectors.getActiveGameIds);

  const resolvedGameId: number | undefined =
    gameId ?? (activeGameIds.length > 0 ? activeGameIds[0] : undefined);

  const game = useAppSelector((state) =>
    resolvedGameId != null ? GameSelectors.getGame(state, resolvedGameId) : undefined,
  );

  return useMemo<CurrentGame>(() => {
    if (!game || resolvedGameId == null) {
      return {
        gameId: resolvedGameId,
        game: undefined,
        localPlayer: undefined,
        isSpectator: false,
        isJudge: false,
        isHost: false,
        isStarted: false,
      };
    }
    const localPlayer = game.players[game.localPlayerId];
    return {
      gameId: resolvedGameId,
      game,
      localPlayer,
      isSpectator: game.spectator,
      isJudge: game.judge,
      isHost: game.hostId === game.localPlayerId,
      isStarted: game.started,
    };
  }, [game, resolvedGameId]);
}
