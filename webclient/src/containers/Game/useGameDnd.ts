import { useCallback, useState } from 'react';
import type { DragEndEvent, DragStartEvent } from '@dnd-kit/core';

import { useWebClient } from '@app/hooks';
import { App, Data } from '@app/types';

export interface GameDnd {
  activeCard: Data.ServerInfo_Card | null;
  handleDragStart: (event: DragStartEvent) => void;
  handleDragEnd: (event: DragEndEvent) => void;
  handleDragCancel: () => void;
}

export interface UseGameDndArgs {
  gameId: number | undefined;
  onDragStart: () => void;
}

export function useGameDnd({ gameId, onDragStart }: UseGameDndArgs): GameDnd {
  const webClient = useWebClient();
  const [activeCard, setActiveCard] = useState<Data.ServerInfo_Card | null>(null);

  const handleDragStart = useCallback(
    (event: DragStartEvent) => {
      const data = event.active.data.current as
        | { card: Data.ServerInfo_Card }
        | undefined;
      setActiveCard(data?.card ?? null);
      // Starting a drag cancels any armed pending-arrow or pending-attach —
      // dnd-kit owns the pointer during the drag, matching desktop where the
      // arrow draw from context menu is aborted if the user grabs a card.
      onDragStart();
    },
    [onDragStart],
  );

  const handleDragEnd = useCallback(
    (event: DragEndEvent) => {
      setActiveCard(null);
      if (!gameId || !event.over || !event.active.data.current) {
        return;
      }
      const source = event.active.data.current as {
        card: Data.ServerInfo_Card;
        sourcePlayerId: number;
        sourceZone: string;
      };
      const target = event.over.data.current as {
        targetPlayerId: number;
        targetZone: string;
        row?: number;
        attachTarget?: boolean;
        targetCardId?: number;
      };

      // Drop onto another card on the table → attach source to target.
      // Desktop's actAttach is only initiated from a table card, so source
      // must also be TABLE. Non-TABLE drops onto a table card fall through
      // to the normal moveCard branch (drop becomes "move to that row").
      if (
        target.attachTarget &&
        target.targetCardId != null &&
        source.sourceZone === App.ZoneName.TABLE
      ) {
        // Guard no-op self-drop (source === target).
        if (
          source.sourcePlayerId === target.targetPlayerId &&
          source.sourceZone === target.targetZone &&
          source.card.id === target.targetCardId
        ) {
          return;
        }
        webClient.request.game.attachCard(gameId, {
          startZone: source.sourceZone,
          cardId: source.card.id,
          targetPlayerId: target.targetPlayerId,
          targetZone: target.targetZone,
          targetCardId: target.targetCardId,
        });
        return;
      }

      const sameZone =
        source.sourcePlayerId === target.targetPlayerId &&
        source.sourceZone === target.targetZone;
      if (sameZone && source.sourceZone === App.ZoneName.TABLE && (source.card.y ?? 0) === (target.row ?? 0)) {
        return;
      }
      if (sameZone && source.sourceZone !== App.ZoneName.TABLE) {
        return;
      }

      webClient.request.game.moveCard(gameId, {
        startPlayerId: source.sourcePlayerId,
        startZone: source.sourceZone,
        cardsToMove: { card: [{ cardId: source.card.id }] },
        targetPlayerId: target.targetPlayerId,
        targetZone: target.targetZone,
        x: 0,
        y: target.row ?? 0,
        isReversed: false,
      });
    },
    [gameId, webClient],
  );

  const handleDragCancel = useCallback(() => {
    setActiveCard(null);
  }, []);

  return { activeCard, handleDragStart, handleDragEnd, handleDragCancel };
}
