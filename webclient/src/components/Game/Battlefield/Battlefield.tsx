import { useMemo } from 'react';
import { App, Data } from '@app/types';
import { GameSelectors, useAppSelector } from '@app/store';
import { useSettings } from '@app/hooks';

import CardSlot from '../CardSlot/CardSlot';
import { makeCardKey } from '../CardRegistry/CardRegistryContext';
import BattlefieldRow from './BattlefieldRow';

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

const ROW_COUNT = 3;

function rowIndexFor(card: Data.ServerInfo_Card): number {
  const y = card.y ?? 0;
  return Math.max(0, Math.min(ROW_COUNT - 1, y));
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
  const cards = useAppSelector((state) =>
    GameSelectors.getCards(state, gameId, playerId, App.ZoneName.TABLE),
  );

  const { value: settings } = useSettings();
  const invertVerticalCoordinate = settings?.invertVerticalCoordinate ?? false;
  // Mirrors desktop TableZone::isInverted() — XOR of per-player mirrored and
  // the global invertVerticalCoordinate preference.
  const isInverted = mirrored !== invertVerticalCoordinate;

  const rows = useMemo<Data.ServerInfo_Card[][]>(() => {
    const bucketed: Data.ServerInfo_Card[][] = Array.from({ length: ROW_COUNT }, () => []);
    for (const card of cards) {
      bucketed[rowIndexFor(card)].push(card);
    }
    for (const row of bucketed) {
      row.sort((a, b) => (a.x ?? 0) - (b.x ?? 0));
    }
    return bucketed;
  }, [cards]);

  const rowOrder = isInverted ? [2, 1, 0] : [0, 1, 2];

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
