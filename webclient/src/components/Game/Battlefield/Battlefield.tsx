import { Data } from '@app/types';

import BattlefieldRow from './BattlefieldRow';
import BattlefieldStackColumn from './BattlefieldStackColumn';
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
  const { rows, stackColumnsByRow, rowOrder, attachmentsByParent } = useBattlefield({
    gameId,
    playerId,
    mirrored,
  });

  return (
    <div className="battlefield" data-testid="battlefield">
      {rowOrder.map((rowIdx) => (
        <BattlefieldRow
          key={rowIdx}
          playerId={playerId}
          row={rowIdx}
          rowCards={rows[rowIdx]}
        >
          {stackColumnsByRow[rowIdx].map((stackCards, colIdx) => {
            if (stackCards == null) {
              // Spacer for an empty stack column so visual position matches
              // stored x — e.g. a single card at x=9 still renders at visual
              // column 3 with cols 0/1/2 empty.
              return (
                <div
                  key={`empty-${rowIdx}-${colIdx}`}
                  className="battlefield__stack-placeholder"
                  data-testid="battlefield-stack-placeholder"
                  data-col={colIdx}
                  aria-hidden="true"
                />
              );
            }
            return (
              // Key on the first (sub-position 0) card's id: React keys must be
              // stable per stack across re-renders, and the leftmost card's id
              // uniquely identifies a stack on this row.
              <BattlefieldStackColumn
                key={stackCards[0].id}
                cards={stackCards}
                attachmentsByParent={attachmentsByParent}
                draggable={canAct}
                ownerPlayerId={playerId}
                arrowSourceKey={arrowSourceKey}
                onCardHover={onCardHover}
                onCardClick={onCardClick}
                onCardContextMenu={onCardContextMenu}
                onCardDoubleClick={onCardDoubleClick}
              />
            );
          })}
        </BattlefieldRow>
      ))}
    </div>
  );
}

export default Battlefield;
