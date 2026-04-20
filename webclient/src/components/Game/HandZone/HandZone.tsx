import { useDroppable } from '@dnd-kit/core';
import { App, Data } from '@app/types';
import { GameSelectors, useAppSelector } from '@app/store';
import { cx } from '@app/utils';

import CardSlot from '../CardSlot/CardSlot';
import { makeCardKey } from '../CardRegistry/CardRegistryContext';

import './HandZone.css';

export interface HandZoneProps {
  gameId: number;
  playerId: number;
  canAct?: boolean;
  arrowSourceKey?: string | null;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onCardClick?: (playerId: number, zone: string, card: Data.ServerInfo_Card) => void;
  onCardContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onZoneContextMenu?: (event: React.MouseEvent) => void;
}

function HandZone({
  gameId,
  playerId,
  canAct = false,
  arrowSourceKey = null,
  onCardHover,
  onCardClick,
  onCardContextMenu,
  onZoneContextMenu,
}: HandZoneProps) {
  const cards = useAppSelector((state) =>
    GameSelectors.getCards(state, gameId, playerId, App.ZoneName.HAND),
  );

  // Match desktop: can't drop into a hand zone that isn't yours (judges
  // aside; server enforces the same restriction). Today only the local
  // HandZone mounts, but this guard future-proofs opponent-hand mirrors.
  const { setNodeRef, isOver } = useDroppable({
    id: `hand-${playerId}`,
    data: { targetPlayerId: playerId, targetZone: App.ZoneName.HAND },
    disabled: !canAct,
  });

  // Right-click anywhere inside the hand that doesn't land on a card opens
  // the hand zone context menu (mulligan / reveal hand). Card-level right-
  // click has its own handler on CardSlot.
  const handleZoneContextMenu = (e: React.MouseEvent<HTMLDivElement>) => {
    if (!onZoneContextMenu) {
      return;
    }
    const target = e.target as HTMLElement;
    if (target.closest('[data-card-id]')) {
      return;
    }
    onZoneContextMenu(e);
  };

  return (
    <div
      ref={setNodeRef}
      className={cx('hand-zone', { 'hand-zone--drop-over': isOver })}
      data-testid="hand-zone"
      onContextMenu={handleZoneContextMenu}
    >
      <div className="hand-zone__label">Hand · {cards.length}</div>
      <div className="hand-zone__cards">
        {cards.map((card) => {
          const key = makeCardKey(playerId, App.ZoneName.HAND, card.id);
          return (
            <CardSlot
              key={card.id}
              card={card}
              draggable={canAct}
              ownerPlayerId={playerId}
              zone={App.ZoneName.HAND}
              isArrowSource={arrowSourceKey === key}
              onMouseEnter={onCardHover}
              onClick={(c) => onCardClick?.(playerId, App.ZoneName.HAND, c)}
              onContextMenu={onCardContextMenu}
            />
          );
        })}
      </div>
    </div>
  );
}

export default HandZone;
