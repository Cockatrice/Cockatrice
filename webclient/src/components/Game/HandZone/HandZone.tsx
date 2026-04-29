import { App, Data } from '@app/types';
import { cx } from '@app/utils';

import CardSlot from '../CardSlot/CardSlot';
import { makeCardKey } from '../CardRegistry/CardRegistryContext';
import { useHandZone } from './useHandZone';

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
  const { cards, setNodeRef, isOver, handleZoneContextMenu } = useHandZone({
    gameId,
    playerId,
    canAct,
    onZoneContextMenu,
  });

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
