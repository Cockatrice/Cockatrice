import { useState } from 'react';

import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';

export interface DeckSelectDialog {
  deckText: string;
  setDeckText: (v: string) => void;
  deckHash: string;
  isReady: boolean;
  canSubmit: boolean;
  canToggleReady: boolean;
  handleSubmitDeck: () => void;
  handleToggleReady: () => void;
}

export function useDeckSelectDialog(gameId: number | undefined): DeckSelectDialog {
  const webClient = useWebClient();
  const localPlayer = useAppSelector((state) =>
    gameId != null ? GameSelectors.getLocalPlayer(state, gameId) : undefined,
  );
  const [deckText, setDeckText] = useState('');

  const deckHash = localPlayer?.properties.deckHash ?? '';
  const isReady = localPlayer?.properties.readyStart ?? false;
  const hasLocalPlayer = localPlayer != null;
  // Guard Submit/Ready on having a local player — today the deckSelectOpen
  // predicate in Game.tsx implies one, but the dialog mounts before the
  // Event_GameJoined echo populates players during reconnect.
  const canSubmit = hasLocalPlayer && deckText.trim().length > 0;
  const canToggleReady = hasLocalPlayer && deckHash.length > 0;

  const handleSubmitDeck = () => {
    if (!canSubmit || gameId == null) {
      return;
    }
    webClient.request.game.deckSelect(gameId, { deck: deckText.trim() });
  };

  const handleToggleReady = () => {
    if (!canToggleReady || gameId == null) {
      return;
    }
    webClient.request.game.readyStart(gameId, { ready: !isReady });
  };

  return {
    deckText,
    setDeckText,
    deckHash,
    isReady,
    canSubmit,
    canToggleReady,
    handleSubmitDeck,
    handleToggleReady,
  };
}
