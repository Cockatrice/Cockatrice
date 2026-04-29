import { GameSelectors, useAppSelector } from '@app/store';
import { App } from '@app/types';

import './StackStrip.css';

export interface StackStripEntry {
  playerId: number;
  name: string;
}

export interface StackStripProps {
  gameId: number;
  entries: StackStripEntry[];
  onZoneClick?: (playerId: number, zoneName: string) => void;
}

interface StackCellProps {
  gameId: number;
  playerId: number;
  name: string;
  onClick?: (playerId: number, zoneName: string) => void;
}

function StackCell({ gameId, playerId, name, onClick }: StackCellProps) {
  const zone = useAppSelector((state) =>
    GameSelectors.getZone(state, gameId, playerId, App.ZoneName.STACK),
  );
  const count = zone?.cardCount ?? 0;
  const clickable = onClick != null;

  const handleClick = () => {
    onClick?.(playerId, App.ZoneName.STACK);
  };

  return (
    <div
      className="stack-strip__cell"
      data-testid={`stack-strip-cell-${playerId}`}
      onClick={clickable ? handleClick : undefined}
      onKeyDown={(e) => {
        if (clickable && (e.key === 'Enter' || e.key === ' ')) {
          e.preventDefault();
          handleClick();
        }
      }}
      role={clickable ? 'button' : undefined}
      tabIndex={clickable ? 0 : undefined}
      aria-label={`${name} stack: ${count} ${count === 1 ? 'card' : 'cards'}`}
    >
      <span className="stack-strip__label">{name}</span>
      <span className="stack-strip__count">{count}</span>
    </div>
  );
}

function StackStrip({ gameId, entries, onZoneClick }: StackStripProps) {
  return (
    <div className="stack-strip" data-testid="stack-strip">
      <span className="stack-strip__heading">Stack</span>
      {entries.map((e) => (
        <StackCell
          key={e.playerId}
          gameId={gameId}
          playerId={e.playerId}
          name={e.name}
          onClick={onZoneClick}
        />
      ))}
    </div>
  );
}

export default StackStrip;
