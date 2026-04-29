import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';

import { useScryfallCard } from '@app/hooks';
import type { Data } from '@app/types';

import { useZoneViewDialog } from './useZoneViewDialog';

import './ZoneViewDialog.css';

export interface ZoneViewDialogProps {
  isOpen: boolean;
  gameId: number | undefined;
  playerId: number | undefined;
  zoneName: string | undefined;
  handleClose: () => void;
  initialPosition?: { x: number; y: number };
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
  const { cards, count, title, position, handlePointerDown, handlePointerMove, handlePointerUp } =
    useZoneViewDialog({ gameId, playerId, zoneName, initialPosition });

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
