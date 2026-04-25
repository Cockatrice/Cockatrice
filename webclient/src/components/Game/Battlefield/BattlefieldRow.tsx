import { ReactNode } from 'react';
import { useDroppable } from '@dnd-kit/core';
import { App, Data } from '@app/types';
import { cx } from '@app/utils';

export interface BattlefieldRowProps {
  playerId: number;
  row: number;
  // Row's current cards (sorted by x, attachments already filtered out). The
  // drop handler reads these off `event.over.data.current` to compute an
  // insertion gridX via gridMath — see useGameDnd.handleDragEnd.
  rowCards: Data.ServerInfo_Card[];
  children: ReactNode;
}

function BattlefieldRow({ playerId, row, rowCards, children }: BattlefieldRowProps) {
  const { setNodeRef, isOver } = useDroppable({
    id: `battlefield-${playerId}-${row}`,
    data: {
      targetPlayerId: playerId,
      targetZone: App.ZoneName.TABLE,
      row,
      rowCards,
    },
  });

  return (
    <div
      ref={setNodeRef}
      className={cx('battlefield__row', { 'battlefield__row--drop-over': isOver })}
      data-row={row}
      data-testid={`battlefield-row-${row}`}
    >
      {children}
    </div>
  );
}

export default BattlefieldRow;
