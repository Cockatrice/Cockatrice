import { useCallback, useState } from 'react';
import type { DragEndEvent, DragStartEvent } from '@dnd-kit/core';

import { useWebClient } from '@app/hooks';
import { App, Data } from '@app/types';
import {
  CARD_HEIGHT_PX,
  CARD_WIDTH_PX,
  MARGIN_LEFT_PX,
  PADDING_X_PX,
  STACKED_CARD_OFFSET_X_PX,
  closestGridPoint,
  mapToGridX,
  stackCountsForRow,
} from '../../components/Game/Battlefield/gridMath';

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

// Derive the drop pointer's x-coordinate relative to the target row's content
// edge. We use the center of the dragged card's translated rect as the "drop
// point" — this matches desktop's behavior where the drop position is the
// card's scene position at release (card_drag_item.cpp:updatePosition), not a
// raw cursor coordinate.
function computePointerXInRow(event: DragEndEvent): number {
  const overRect = event.over?.rect;
  const activeRect = event.active.rect.current.translated;
  if (!overRect || !activeRect) {
    return 0;
  }
  const cardCenterX = activeRect.left + activeRect.width / 2;
  return cardCenterX - overRect.left - MARGIN_LEFT_PX;
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
        rowCards?: Data.ServerInfo_Card[];
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
      // Non-TABLE same-zone drops aren't meaningful (dragging within the hand
      // or library isn't a user-facing reorder on desktop either).
      if (sameZone && source.sourceZone !== App.ZoneName.TABLE) {
        return;
      }

      const targetRow = target.row ?? 0;
      const targetIsTable = target.targetZone === App.ZoneName.TABLE;
      const sameRowOnTable =
        sameZone && source.sourceZone === App.ZoneName.TABLE && (source.card.y ?? 0) === targetRow;

      // Compute an integer gridX matching desktop's stack-and-subposition
      // packing (table_zone.cpp:mapToGrid + closestGridPoint). Non-TABLE
      // targets get x = 0 — the server assigns a position in piles/hand.
      let gridX = 0;
      if (targetIsTable) {
        const rowCards = target.rowCards ?? [];
        // When reordering within the same row, exclude the dragged card from
        // occupancy / stack width — otherwise closestGridPoint would skip its
        // own slot and a same-position no-op drop would bump to base+1.
        const neighbors = sameRowOnTable
          ? rowCards.filter((c) => c.id !== source.card.id)
          : rowCards;
        const pointerXInRow = computePointerXInRow(event);
        const stackCounts = stackCountsForRow(neighbors);
        // Cards render at laneHeight × (146/204) via CSS aspect-ratio. Derive
        // the same effective width here so gridMath maps the pointer to the
        // stack/sub-position the user visually dropped on — at any zoom level.
        const laneHeight = event.over.rect.height;
        const effectiveCardWidth = laneHeight > 0
          ? (laneHeight * CARD_WIDTH_PX) / CARD_HEIGHT_PX
          : CARD_WIDTH_PX;
        const effectiveOffsetX =
          (effectiveCardWidth * STACKED_CARD_OFFSET_X_PX) / CARD_WIDTH_PX;
        const rawGridX = mapToGridX(
          pointerXInRow,
          stackCounts,
          effectiveCardWidth,
          effectiveOffsetX,
          PADDING_X_PX,
        );
        const occupied = new Set(neighbors.map((c) => c.x ?? 0));
        const resolved = closestGridPoint(rawGridX, occupied);
        // Fully-occupied 3-card stack → desktop silently rejects (see
        // card_drag_item.cpp:115); skip dispatch to match.
        if (resolved == null) {
          return;
        }
        gridX = resolved;
      }

      // targetRow passed from BattlefieldRow is already the LOGICAL y (cards
      // get y=rowIdx where rowIdx iterates rowOrder = [0,1,2] or [2,1,0]). On
      // a mirrored board the first iterated rowIdx is 2, so dropping on the
      // visual top row yields target.row=2 — which is the correct server-side
      // y. Unlike desktop's mapToGrid (which takes pixel-y and inverts), we
      // never see a pre-inverted value here, so no re-inversion needed.
      webClient.request.game.moveCard(gameId, {
        startPlayerId: source.sourcePlayerId,
        startZone: source.sourceZone,
        cardsToMove: { card: [{ cardId: source.card.id }] },
        targetPlayerId: target.targetPlayerId,
        targetZone: target.targetZone,
        x: gridX,
        y: targetIsTable ? targetRow : 0,
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
