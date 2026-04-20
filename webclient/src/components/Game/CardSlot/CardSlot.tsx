import type { Data } from '@app/types';
import { cx } from '@app/utils';

import { useCardSlot } from './useCardSlot';

import './CardSlot.css';

export interface CardSlotProps {
  card: Data.ServerInfo_Card;
  inverted?: boolean;
  draggable?: boolean;
  isArrowSource?: boolean;
  /** The player that owns this card (matches desktop's `getOwner()`). Kept
   *  as `ownerPlayerId`, not `sourcePlayerId`, because it reflects the card
   *  in the game state rather than any drag origin. */
  ownerPlayerId?: number;
  zone?: string;
  onClick?: (card: Data.ServerInfo_Card) => void;
  onDoubleClick?: (card: Data.ServerInfo_Card) => void;
  onContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onMouseEnter?: (card: Data.ServerInfo_Card) => void;
}

function CardSlot({
  card,
  inverted = false,
  draggable = false,
  isArrowSource = false,
  ownerPlayerId,
  zone,
  onClick,
  onDoubleClick,
  onContextMenu,
  onMouseEnter,
}: CardSlotProps) {
  const { smallUrl, attributes, listeners, isDragging, isOver, rootRef } = useCardSlot({
    card,
    draggable,
    ownerPlayerId,
    zone,
  });

  const className = cx('card-slot', {
    'card-slot--tapped': card.tapped,
    'card-slot--inverted': inverted,
    'card-slot--face-down': card.faceDown,
    'card-slot--attacking': card.attacking,
    'card-slot--dragging': isDragging,
    'card-slot--arrow-source': isArrowSource,
    'card-slot--attach-over': isOver,
  });

  return (
    <div
      ref={rootRef}
      className={className}
      onClick={() => onClick?.(card)}
      onDoubleClick={() => onDoubleClick?.(card)}
      onContextMenu={(e) => onContextMenu?.(card, e)}
      onMouseEnter={() => onMouseEnter?.(card)}
      data-testid="card-slot"
      data-card-id={card.id}
      data-card-owner={ownerPlayerId ?? ''}
      data-card-zone={zone ?? ''}
      {...(draggable ? attributes : {})}
      {...(draggable ? listeners : {})}
    >
      {card.faceDown ? (
        <div className="card-slot__back" aria-label="face-down card" />
      ) : (
        smallUrl && (
          <img className="card-slot__image" src={smallUrl} alt={card.name} />
        )
      )}

      {card.annotation && !card.faceDown && (
        <div className="card-slot__annotation">{card.annotation}</div>
      )}

      {card.pt && !card.faceDown && (
        <div className="card-slot__pt">{card.pt}</div>
      )}

      {card.counterList.length > 0 && !card.faceDown && (
        <div className="card-slot__counters">
          {card.counterList.map((c) => (
            <span key={c.id} className="card-slot__counter">
              {c.value}
            </span>
          ))}
        </div>
      )}
    </div>
  );
}

export default CardSlot;
