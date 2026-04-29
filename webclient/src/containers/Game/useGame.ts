import { RefObject, useCallback, useMemo, useRef, useState } from 'react';
import { KeyboardSensor, PointerSensor, useSensor, useSensors } from '@dnd-kit/core';

import { createCardRegistry, type CardRegistry } from '@app/components';
import { useCurrentGame, useGameAccess, type CurrentGame, type GameAccess } from '@app/hooks';
import type { Data } from '@app/types';

import { useGameArrowInteractions, type GameArrowInteractions } from './useGameArrowInteractions';
import { useGameDialogs, type GameDialogs } from './useGameDialogs';
import { useGameDnd, type GameDnd } from './useGameDnd';
import { useGameLifecycleNavigation } from './useGameLifecycleNavigation';
import { useGameOpponentSelector, type GameOpponentSelector } from './useGameOpponentSelector';

export interface Game extends CurrentGame {
  boardRef: RefObject<HTMLDivElement>;
  cardRegistry: CardRegistry;
  sensors: ReturnType<typeof useSensors>;
  hoveredCard: Data.ServerInfo_Card | null;
  setHoveredCard: (card: Data.ServerInfo_Card | null) => void;
  isRotated: boolean;
  toggleRotated: () => void;
  localAccess: GameAccess;
  opponentAccess: GameAccess;
  deckSelectOpen: boolean;
  opponents: GameOpponentSelector;
  arrows: GameArrowInteractions;
  dialogs: GameDialogs;
  dnd: GameDnd;
}

export function useGame(): Game {
  const current = useCurrentGame();
  const { gameId, game, localPlayer, isSpectator } = current;

  useGameLifecycleNavigation(gameId);

  const boardRef = useRef<HTMLDivElement>(null);
  const cardRegistry = useMemo(() => createCardRegistry(), []);
  const sensors = useSensors(useSensor(PointerSensor), useSensor(KeyboardSensor));
  const [hoveredCard, setHoveredCard] = useState<Data.ServerInfo_Card | null>(null);
  // View-only 90° rotation; local to this tab, mirrors desktop's
  // Player::actRotateLocal which applies a QGraphicsView transform with no
  // server call.
  const [isRotated, setIsRotated] = useState(false);
  const toggleRotated = useCallback(() => setIsRotated((prev) => !prev), []);

  const opponents = useGameOpponentSelector(game);
  const localAccess = useGameAccess(gameId, game?.localPlayerId);
  const opponentAccess = useGameAccess(gameId, opponents.shownOpponentId);

  const arrows = useGameArrowInteractions({ gameId, game, boardRef, cardRegistry });
  const dialogs = useGameDialogs({
    gameId,
    game,
    localPlayer,
    localAccess,
    isSpectator,
    startPendingArrow: arrows.startPendingArrow,
    startPendingAttach: arrows.startPendingAttach,
  });
  const dnd = useGameDnd({ gameId, onDragStart: arrows.cancelPendingOnDragStart });

  // Explicit localPlayer null-check closes a window during reconnect where
  // `game` is present but `players[localPlayerId]` is not yet populated
  // (Event_GameStateChanged arrives after Event_GameJoined echo).
  const deckSelectOpen =
    game != null &&
    localPlayer != null &&
    !game.started &&
    !current.isSpectator &&
    !current.isJudge &&
    !localPlayer.properties.readyStart;

  return {
    ...current,
    boardRef,
    cardRegistry,
    sensors,
    hoveredCard,
    setHoveredCard,
    isRotated,
    toggleRotated,
    localAccess,
    opponentAccess,
    deckSelectOpen,
    opponents,
    arrows,
    dialogs,
    dnd,
  };
}
