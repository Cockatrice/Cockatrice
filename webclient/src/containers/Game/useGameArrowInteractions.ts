import { RefObject, useCallback, useEffect, useMemo, useRef, useState } from 'react';

import type { CardRegistry } from '@app/components';
import { makeCardKey } from '@app/components';
import { useWebClient } from '@app/hooks';
import { App, Data, type Enriched } from '@app/types';

interface PendingArrow {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
}

// Shares shape with PendingArrow today, but kept distinct so future
// protocol fields (e.g. desktop's attach-target coord hints) can diverge
// without a runtime switch.
interface PendingAttach {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
}

interface ArrowDragState {
  sourcePlayerId: number;
  sourceZone: string;
  sourceCardId: number;
  startX: number;
  startY: number;
  currentX: number;
  currentY: number;
  moved: boolean;
}

export interface ArrowDragPreview {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
  color: string;
}

export interface GameArrowInteractions {
  arrowSourceKey: string | null;
  dragPreview: ArrowDragPreview | null;
  handleBoardMouseDown: (e: React.MouseEvent<HTMLDivElement>) => void;
  handleCardClick: (
    ownerPlayerId: number,
    zone: string,
    card: Data.ServerInfo_Card,
  ) => void;
  handleCardDoubleClick: (sourceZone: string, card: Data.ServerInfo_Card) => void;
  startPendingArrow: (source: PendingArrow) => void;
  startPendingAttach: (source: PendingAttach) => void;
  cancelPendingOnDragStart: () => void;
}

export interface UseGameArrowInteractionsArgs {
  gameId: number | undefined;
  game: Enriched.GameEntry | undefined;
  boardRef: RefObject<HTMLDivElement>;
  cardRegistry: CardRegistry;
}

function arrowColorForModifiers(e: {
  ctrlKey: boolean;
  altKey: boolean;
  shiftKey: boolean;
}): App.ColorRGBA {
  if (e.ctrlKey) {
    return App.ArrowColor.YELLOW;
  }
  if (e.altKey) {
    return App.ArrowColor.BLUE;
  }
  if (e.shiftKey) {
    return App.ArrowColor.GREEN;
  }
  return App.ArrowColor.RED;
}

const ARROW_DRAG_THRESHOLD_PX = 8;

export function useGameArrowInteractions({
  gameId,
  game,
  boardRef,
  cardRegistry,
}: UseGameArrowInteractionsArgs): GameArrowInteractions {
  const webClient = useWebClient();

  const [pendingArrow, setPendingArrow] = useState<PendingArrow | null>(null);
  const [pendingAttach, setPendingAttach] = useState<PendingAttach | null>(null);
  const [arrowDrag, setArrowDrag] = useState<ArrowDragState | null>(null);
  const suppressNextContextMenuRef = useRef(false);

  // ESC cancels a pending arrow OR attach (matches desktop). Suppress the
  // cancel when a MUI dialog is open — the dialog's own ESC handler should
  // win so the user isn't rug-pulled out of a modal form.
  useEffect(() => {
    if (!pendingArrow && !pendingAttach && !arrowDrag) {
      return undefined;
    }
    const handler = (e: KeyboardEvent) => {
      if (e.key !== 'Escape') {
        return;
      }
      if (document.querySelector('.MuiDialog-root[role="dialog"]')) {
        return;
      }
      setPendingArrow(null);
      setPendingAttach(null);
      setArrowDrag(null);
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [pendingArrow, pendingAttach, arrowDrag]);

  // Right-click-drag arrow-draw lifecycle: window-level mousemove + mouseup
  // listeners that track the cursor and finalize on release.
  useEffect(() => {
    if (!arrowDrag) {
      return undefined;
    }

    const handleMove = (e: MouseEvent) => {
      setArrowDrag((prev) => {
        if (!prev) {
          return prev;
        }
        const movedX = Math.abs(e.clientX - prev.startX);
        const movedY = Math.abs(e.clientY - prev.startY);
        const moved = prev.moved || movedX + movedY > ARROW_DRAG_THRESHOLD_PX;
        return { ...prev, currentX: e.clientX, currentY: e.clientY, moved };
      });
    };

    const handleUp = (e: MouseEvent) => {
      if (e.button !== 2) {
        return;
      }
      const drag = arrowDrag;
      if (!drag) {
        return;
      }
      const movedX = Math.abs(e.clientX - drag.startX);
      const movedY = Math.abs(e.clientY - drag.startY);
      const moved = drag.moved || movedX + movedY > ARROW_DRAG_THRESHOLD_PX;
      setArrowDrag(null);
      if (!moved || gameId == null) {
        // Short right-click with no drag: let the contextmenu handler run
        // (it will open the card menu).
        return;
      }
      // Any real drag suppresses the contextmenu event that follows mouseup.
      suppressNextContextMenuRef.current = true;

      const el = document.elementFromPoint(e.clientX, e.clientY)?.closest('[data-card-id]') as HTMLElement | null;
      if (!el) {
        return;
      }
      const targetPlayerId = Number(el.getAttribute('data-card-owner'));
      const targetZone = el.getAttribute('data-card-zone') ?? '';
      const targetCardId = Number(el.getAttribute('data-card-id'));
      if (!Number.isFinite(targetPlayerId) || !targetZone || !Number.isFinite(targetCardId)) {
        return;
      }
      // Same-card drops are cancellations.
      if (
        targetPlayerId === drag.sourcePlayerId &&
        targetZone === drag.sourceZone &&
        targetCardId === drag.sourceCardId
      ) {
        return;
      }
      // Desktop parity: dragging an arrow from a local-hand card to a target
      // outside the hand auto-plays the card (card_item.cpp:243-250) — the
      // card is moved to the battlefield before any arrow is drawn. The
      // server re-keys the card id during the move, so we can't also send
      // createArrow here; instead we resolve this drag as a play-card intent.
      if (
        drag.sourceZone === App.ZoneName.HAND &&
        drag.sourcePlayerId === game?.localPlayerId &&
        targetZone !== App.ZoneName.HAND
      ) {
        webClient.request.game.moveCard(gameId, {
          startPlayerId: drag.sourcePlayerId,
          startZone: drag.sourceZone,
          cardsToMove: { card: [{ cardId: drag.sourceCardId }] },
          targetPlayerId: drag.sourcePlayerId,
          targetZone: App.ZoneName.TABLE,
          x: 0,
          y: 0,
          isReversed: false,
        });
        return;
      }
      webClient.request.game.createArrow(gameId, {
        startPlayerId: drag.sourcePlayerId,
        startZone: drag.sourceZone,
        startCardId: drag.sourceCardId,
        targetPlayerId,
        targetZone,
        targetCardId,
        arrowColor: arrowColorForModifiers(e),
      });
    };

    window.addEventListener('mousemove', handleMove);
    window.addEventListener('mouseup', handleUp);
    return () => {
      window.removeEventListener('mousemove', handleMove);
      window.removeEventListener('mouseup', handleUp);
    };
  }, [arrowDrag, gameId, webClient, game?.localPlayerId]);

  // Suppress the browser contextmenu event after a right-drag.
  useEffect(() => {
    const handler = (e: MouseEvent) => {
      if (suppressNextContextMenuRef.current) {
        e.preventDefault();
        suppressNextContextMenuRef.current = false;
      }
    };
    window.addEventListener('contextmenu', handler);
    return () => window.removeEventListener('contextmenu', handler);
  }, []);

  const handleBoardMouseDown = useCallback((e: React.MouseEvent<HTMLDivElement>) => {
    if (e.button !== 2) {
      return;
    }
    const el = (e.target as HTMLElement).closest('[data-card-id]') as HTMLElement | null;
    if (!el) {
      return;
    }
    const sourcePlayerId = Number(el.getAttribute('data-card-owner'));
    const sourceZone = el.getAttribute('data-card-zone') ?? '';
    const sourceCardId = Number(el.getAttribute('data-card-id'));
    if (!Number.isFinite(sourcePlayerId) || !sourceZone || !Number.isFinite(sourceCardId)) {
      return;
    }
    setArrowDrag({
      sourcePlayerId,
      sourceZone,
      sourceCardId,
      startX: e.clientX,
      startY: e.clientY,
      currentX: e.clientX,
      currentY: e.clientY,
      moved: false,
    });
  }, []);

  const arrowSourceKey = pendingArrow
    ? makeCardKey(pendingArrow.sourcePlayerId, pendingArrow.sourceZone, pendingArrow.sourceCardId)
    : pendingAttach
      ? makeCardKey(pendingAttach.sourcePlayerId, pendingAttach.sourceZone, pendingAttach.sourceCardId)
      : arrowDrag
        ? makeCardKey(arrowDrag.sourcePlayerId, arrowDrag.sourceZone, arrowDrag.sourceCardId)
        : null;

  // Convert arrowDrag's viewport coords → board-relative coords for the SVG
  // preview line. Recomputed every render; cheap.
  const dragPreview = useMemo<ArrowDragPreview | null>(() => {
    if (!arrowDrag || !arrowDrag.moved) {
      return null;
    }
    const boardRect = boardRef.current?.getBoundingClientRect();
    const sourceEl = cardRegistry.get(
      makeCardKey(arrowDrag.sourcePlayerId, arrowDrag.sourceZone, arrowDrag.sourceCardId),
    );
    if (!boardRect || !sourceEl) {
      return null;
    }
    const sourceRect = sourceEl.getBoundingClientRect();
    return {
      x1: sourceRect.left + sourceRect.width / 2 - boardRect.left,
      y1: sourceRect.top + sourceRect.height / 2 - boardRect.top,
      x2: arrowDrag.currentX - boardRect.left,
      y2: arrowDrag.currentY - boardRect.top,
      color: App.rgbaToCss(App.ArrowColor.RED),
    };
  }, [arrowDrag, cardRegistry, boardRef]);

  const handleCardClick = useCallback(
    (ownerPlayerId: number, zone: string, card: Data.ServerInfo_Card) => {
      if (gameId == null) {
        return;
      }

      // Pending-attach (from CardContextMenu "Attach to card…") takes
      // precedence over pending-arrow because it was activated by a later menu
      // action. Click on the pending source to cancel.
      if (pendingAttach) {
        if (
          pendingAttach.sourcePlayerId === ownerPlayerId &&
          pendingAttach.sourceZone === zone &&
          pendingAttach.sourceCardId === card.id
        ) {
          setPendingAttach(null);
          return;
        }
        webClient.request.game.attachCard(gameId, {
          startZone: pendingAttach.sourceZone,
          cardId: pendingAttach.sourceCardId,
          targetPlayerId: ownerPlayerId,
          targetZone: zone,
          targetCardId: card.id,
        });
        setPendingAttach(null);
        return;
      }

      if (!pendingArrow) {
        return;
      }
      // Cancel if user re-clicks the pending source.
      if (
        pendingArrow.sourcePlayerId === ownerPlayerId &&
        pendingArrow.sourceZone === zone &&
        pendingArrow.sourceCardId === card.id
      ) {
        setPendingArrow(null);
        return;
      }
      // Desktop parity: arrow from local-hand → non-hand target auto-plays the
      // card (card_item.cpp:243-250). The server re-keys the moved card id, so
      // we resolve this as a play-card intent and drop the arrow command.
      if (
        pendingArrow.sourceZone === App.ZoneName.HAND &&
        pendingArrow.sourcePlayerId === game?.localPlayerId &&
        zone !== App.ZoneName.HAND
      ) {
        webClient.request.game.moveCard(gameId, {
          startPlayerId: pendingArrow.sourcePlayerId,
          startZone: pendingArrow.sourceZone,
          cardsToMove: { card: [{ cardId: pendingArrow.sourceCardId }] },
          targetPlayerId: pendingArrow.sourcePlayerId,
          targetZone: App.ZoneName.TABLE,
          x: 0,
          y: 0,
          isReversed: false,
        });
        setPendingArrow(null);
        return;
      }
      webClient.request.game.createArrow(gameId, {
        startPlayerId: pendingArrow.sourcePlayerId,
        startZone: pendingArrow.sourceZone,
        startCardId: pendingArrow.sourceCardId,
        targetPlayerId: ownerPlayerId,
        targetZone: zone,
        targetCardId: card.id,
        arrowColor: App.ArrowColor.RED,
      });
      setPendingArrow(null);
    },
    [gameId, game?.localPlayerId, pendingArrow, pendingAttach, webClient],
  );

  const handleCardDoubleClick = useCallback(
    (sourceZone: string, card: Data.ServerInfo_Card) => {
      if (sourceZone !== App.ZoneName.TABLE || gameId == null) {
        return;
      }
      // Desktop's arrow drag owns the pointer while active; mirror that by
      // short-circuiting tap-toggle while a pending arrow/attach is armed.
      if (pendingArrow || pendingAttach) {
        return;
      }
      webClient.request.game.setCardAttr(gameId, {
        zone: sourceZone,
        cardId: card.id,
        attribute: Data.CardAttribute.AttrTapped,
        attrValue: card.tapped ? '0' : '1',
      });
    },
    [gameId, pendingArrow, pendingAttach, webClient],
  );

  const startPendingArrow = useCallback((source: PendingArrow) => {
    setPendingArrow(source);
  }, []);

  const startPendingAttach = useCallback((source: PendingAttach) => {
    setPendingAttach(source);
  }, []);

  const cancelPendingOnDragStart = useCallback(() => {
    setPendingArrow((prev) => (prev ? null : prev));
    setPendingAttach((prev) => (prev ? null : prev));
  }, []);

  return {
    arrowSourceKey,
    dragPreview,
    handleBoardMouseDown,
    handleCardClick,
    handleCardDoubleClick,
    startPendingArrow,
    startPendingAttach,
    cancelPendingOnDragStart,
  };
}
