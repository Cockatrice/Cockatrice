import { App, Data } from '@app/types';

import CardSlot from '../CardSlot/CardSlot';
import { makeCardKey } from '../CardRegistry/CardRegistryContext';
import BattlefieldRow from './BattlefieldRow';
import { useBattlefield } from './useBattlefield';

import './Battlefield.css';

export interface BattlefieldProps {
  gameId: number;
  playerId: number;
  mirrored?: boolean;
  canAct?: boolean;
  arrowSourceKey?: string | null;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onCardClick?: (playerId: number, zone: string, card: Data.ServerInfo_Card) => void;
  onCardContextMenu?: (card: Data.ServerInfo_Card, event: React.MouseEvent) => void;
  onCardDoubleClick?: (card: Data.ServerInfo_Card) => void;
}

function Battlefield({
  gameId,
  playerId,
  mirrored = false,
  canAct = false,
  arrowSourceKey = null,
  onCardHover,
  onCardClick,
  onCardContextMenu,
  onCardDoubleClick,
}: BattlefieldProps) {
  const { rows, rowOrder, isInverted } = useBattlefield({ gameId, playerId, mirrored });

  return (
    <div className="battlefield" data-testid="battlefield">
      {rowOrder.map((rowIdx) => (
        <BattlefieldRow key={rowIdx} playerId={playerId} row={rowIdx}>
          {rows[rowIdx].map((card) => {
            const key = makeCardKey(playerId, App.ZoneName.TABLE, card.id);
            return (
              <CardSlot
                key={card.id}
                card={card}
                inverted={isInverted}
                draggable={canAct}
                ownerPlayerId={playerId}
                zone={App.ZoneName.TABLE}
                isArrowSource={arrowSourceKey === key}
                onMouseEnter={onCardHover}
                onClick={(c) => onCardClick?.(playerId, App.ZoneName.TABLE, c)}
                onContextMenu={onCardContextMenu}
                onDoubleClick={onCardDoubleClick}
              />
            );
          })}
        </BattlefieldRow>
      ))}
    </div>
  );
}

export default Battlefield;
