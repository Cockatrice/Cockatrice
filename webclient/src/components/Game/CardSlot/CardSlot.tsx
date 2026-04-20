import { useCallback, useId } from 'react';
import { useDraggable, useDroppable } from '@dnd-kit/core';

import { useScryfallCard } from '@app/hooks';
import { App } from '@app/types';
import type { Data } from '@app/types';
import { cx } from '@app/utils';

import { makeCardKey, useRegisterCardRef } from '../CardRegistry/CardRegistryContext';

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
  const { smallUrl } = useScryfallCard(card);

  // React-stable id salts the dnd-kit IDs so even two disabled CardSlots
  // rendering the same card (during state transitions / hidden-zone leaks)
  // never collide. Without the salt, pre-owner/zone render cycles shared
  // `card-x-x-<id>` and dnd-kit warned.
  const instanceId = useId();

  const { attributes, listeners, setNodeRef, isDragging } = useDraggable({
    id: `card-${ownerPlayerId ?? instanceId}-${zone ?? 'x'}-${card.id}`,
    data: { card, sourcePlayerId: ownerPlayerId, sourceZone: zone },
    disabled: !draggable || ownerPlayerId == null || zone == null,
  });

  // Cards on the battlefield double as drop targets for drag-to-attach.
  // Other zones don't support attach (desktop's Player::actAttach rejects
  // non-table targets), so the droppable is only live for TABLE.
  const droppableEnabled =
    ownerPlayerId != null && zone === App.ZoneName.TABLE;
  const { setNodeRef: setDropRef, isOver } = useDroppable({
    id: `card-drop-${ownerPlayerId ?? instanceId}-${zone ?? 'x'}-${card.id}`,
    data: {
      attachTarget: true,
      targetPlayerId: ownerPlayerId,
      targetZone: zone,
      targetCardId: card.id,
    },
    disabled: !droppableEnabled,
  });

  const registryKey =
    ownerPlayerId != null && zone != null
      ? makeCardKey(ownerPlayerId, zone, card.id)
      : null;
  const registerRef = useRegisterCardRef(registryKey);

  const rootRef = useCallback(
    (el: HTMLElement | null) => {
      registerRef(el);
      if (draggable) {
        setNodeRef(el);
      }
      if (droppableEnabled) {
        setDropRef(el);
      }
    },
    [registerRef, setNodeRef, setDropRef, draggable, droppableEnabled],
  );

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
