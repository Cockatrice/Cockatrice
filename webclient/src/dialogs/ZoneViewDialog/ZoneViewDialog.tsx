import { useRef, useState } from 'react';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';

import { useScryfallCard } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Data } from '@app/types';

import './ZoneViewDialog.css';

const EMPTY_CARDS: Data.ServerInfo_Card[] = [];

export interface ZoneViewDialogProps {
  isOpen: boolean;
  gameId: number | undefined;
  playerId: number | undefined;
  zoneName: string | undefined;
  handleClose: () => void;
  initialPosition?: { x: number; y: number };
}

function zoneLabel(zoneName: string | undefined): string {
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

function ZoneThumbnail({ card }: { card: Data.ServerInfo_Card }) {
  const { smallUrl } = useScryfallCard(card);
  return (
    <div className="zone-view-dialog__card" data-testid={`zone-view-card-${card.id}`}>
      {smallUrl && !card.faceDown ? (
        <img src={smallUrl} alt={card.name} className="zone-view-dialog__card-image" />
      ) : (
        <div className="zone-view-dialog__card-placeholder">
          {card.faceDown ? 'Face Down' : card.name}
        </div>
      )}
    </div>
  );
}

const DEFAULT_POSITION = { x: 80, y: 80 };

function ZoneViewDialog({
  isOpen,
  gameId,
  playerId,
  zoneName,
  handleClose,
  initialPosition = DEFAULT_POSITION,
}: ZoneViewDialogProps) {
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

  if (!isOpen) {
    return null;
  }

  return (
    <div
      className="zone-view-dialog"
      role="dialog"
      aria-label={title}
      data-testid="zone-view-dialog"
      style={{ left: position.x, top: position.y }}
    >
      <div
        className="zone-view-dialog__header"
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
        onPointerCancel={handlePointerUp}
      >
        <span className="zone-view-dialog__title">{title}</span>
        <IconButton
          onClick={handleClose}
          size="small"
          aria-label="close zone view"
          className="zone-view-dialog__close"
        >
          <CloseIcon fontSize="small" />
        </IconButton>
      </div>
      <div className="zone-view-dialog__body">
        {cards.length === 0 ? (
          <div className="zone-view-dialog__empty">
            {count > 0
              ? `${count} hidden card${count === 1 ? '' : 's'}`
              : 'This zone is empty.'}
          </div>
        ) : (
          <div className="zone-view-dialog__grid">
            {cards.map((card) => (
              <ZoneThumbnail key={card.id} card={card} />
            ))}
          </div>
        )}
      </div>
    </div>
  );
}

export default ZoneViewDialog;
