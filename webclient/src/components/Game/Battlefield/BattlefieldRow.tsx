import { ReactNode } from 'react';
import { useDroppable } from '@dnd-kit/core';
import { App } from '@app/types';
import { cx } from '@app/utils';

export interface BattlefieldRowProps {
  playerId: number;
  row: number;
  children: ReactNode;
}

function BattlefieldRow({ playerId, row, children }: BattlefieldRowProps) {
  const { setNodeRef, isOver } = useDroppable({
    id: `battlefield-${playerId}-${row}`,
    data: { targetPlayerId: playerId, targetZone: App.ZoneName.TABLE, row },
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
