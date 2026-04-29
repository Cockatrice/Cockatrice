import { useMemo } from 'react';
import { GameSelectors, useAppSelector } from '@app/store';

export interface GameAccess {
  canAct: boolean;
  canView: boolean;
  isLocalPlayer: boolean;
}

/**
 * Per-action gate: returns whether the current user may issue commands
 * affecting `targetPlayerId` (or the game as a whole when omitted).
 */
export function useGameAccess(gameId: number | undefined, targetPlayerId?: number): GameAccess {
  const game = useAppSelector((state) =>
    gameId != null ? GameSelectors.getGame(state, gameId) : undefined,
  );

  return useMemo<GameAccess>(() => {
    if (!game) {
      return { canAct: false, canView: false, isLocalPlayer: false };
    }
    const isLocalPlayer = targetPlayerId != null && targetPlayerId === game.localPlayerId;
    const noTargetButLocalControls = targetPlayerId == null && !game.spectator;
    const canAct = game.judge || (!game.spectator && (isLocalPlayer || noTargetButLocalControls));

    return {
      canAct,
      canView: true,
      isLocalPlayer,
    };
  }, [game, targetPlayerId]);
}
