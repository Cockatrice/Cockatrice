import { useRef, useState } from 'react';

import { GameSelectors, useAppSelector } from '@app/store';
import type { Data } from '@app/types';

const EMPTY_CARDS: Data.ServerInfo_Card[] = [];

export interface ZoneViewDialog {
  cards: Data.ServerInfo_Card[];
  count: number;
  title: string;
  position: { x: number; y: number };
  handlePointerDown: (e: React.PointerEvent<HTMLDivElement>) => void;
  handlePointerMove: (e: React.PointerEvent<HTMLDivElement>) => void;
  handlePointerUp: (e: React.PointerEvent<HTMLDivElement>) => void;
}

export interface UseZoneViewDialogArgs {
  gameId: number | undefined;
  playerId: number | undefined;
  zoneName: string | undefined;
  initialPosition: { x: number; y: number };
}

export function zoneLabel(zoneName: string | undefined): string {
  switch (zoneName) {
    case 'grave': return 'Graveyard';
    case 'rfg': return 'Exile';
    case 'deck': return 'Library';
    case 'sb': return 'Sideboard';
    case 'stack': return 'Stack';
    case 'hand': return 'Hand';
    case 'table': return 'Battlefield';
    default: return zoneName ?? '';
  }
}

export function useZoneViewDialog({
  gameId,
  playerId,
  zoneName,
  initialPosition,
}: UseZoneViewDialogArgs): ZoneViewDialog {
  const cards = useAppSelector((state) =>
    gameId != null && playerId != null && zoneName != null
      ? GameSelectors.getCards(state, gameId, playerId, zoneName)
      : EMPTY_CARDS,
  );
  const zone = useAppSelector((state) =>
    gameId != null && playerId != null && zoneName != null
      ? GameSelectors.getZone(state, gameId, playerId, zoneName)
      : undefined,
  );
  const playerName = useAppSelector((state) => {
    if (gameId == null || playerId == null) {
      return undefined;
    }
    return GameSelectors.getPlayer(state, gameId, playerId)?.properties.userInfo?.name;
  });

  const count = zone?.cardCount ?? cards.length;
  const title = `${playerName ?? ''} ${zoneLabel(zoneName)} (${count})`.trim();

  // initialPosition is a caller-provided spawn point; we only honor it on mount.
  // Later rerenders of the parent must not clobber a user's drag-positioned panel.
  const [position, setPosition] = useState(initialPosition);
  const dragStateRef = useRef<{
    pointerId: number;
    originX: number;
    originY: number;
    panelX: number;
    panelY: number;
  } | null>(null);

  const handlePointerDown = (e: React.PointerEvent<HTMLDivElement>) => {
    if (e.button !== 0) {
      return;
    }
    const target = e.currentTarget;
    target.setPointerCapture(e.pointerId);
    dragStateRef.current = {
      pointerId: e.pointerId,
      originX: e.clientX,
      originY: e.clientY,
      panelX: position.x,
      panelY: position.y,
    };
  };

  const handlePointerMove = (e: React.PointerEvent<HTMLDivElement>) => {
    const drag = dragStateRef.current;
    if (!drag || e.pointerId !== drag.pointerId) {
      return;
    }
    setPosition({
      x: drag.panelX + (e.clientX - drag.originX),
      y: drag.panelY + (e.clientY - drag.originY),
    });
  };

  const handlePointerUp = (e: React.PointerEvent<HTMLDivElement>) => {
    const drag = dragStateRef.current;
    if (!drag || e.pointerId !== drag.pointerId) {
      return;
    }
    e.currentTarget.releasePointerCapture(e.pointerId);
    dragStateRef.current = null;
  };

  return { cards, count, title, position, handlePointerDown, handlePointerMove, handlePointerUp };
}
