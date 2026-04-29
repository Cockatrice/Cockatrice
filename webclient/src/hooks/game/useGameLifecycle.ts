import { GameTypes } from '@app/store';

import { useReduxEffect } from '../useReduxEffect';

export interface GameLifecycleHandlers {
  onKicked: () => void;
  onGameClosed: () => void;
}

/**
 * Watches for `kicked` / `gameClosed` events targeting `gameId` and invokes
 * the matching handler. Consumers own toast + navigation because hooks cannot
 * import from `components/`. Mirrors desktop Cockatrice's TabGame tear-down:
 * game_event_handler.cpp eventKicked / eventGameClosed emit signals that close
 * the tab.
 */
export function useGameLifecycle(
  gameId: number | undefined,
  handlers: GameLifecycleHandlers,
): void {
  useReduxEffect(
    (action) => {
      if (gameId == null || action.payload?.gameId !== gameId) {
        return;
      }
      if (action.type === GameTypes.KICKED) {
        handlers.onKicked();
      } else {
        handlers.onGameClosed();
      }
    },
    [GameTypes.KICKED, GameTypes.GAME_CLOSED],
    [gameId],
  );
}
