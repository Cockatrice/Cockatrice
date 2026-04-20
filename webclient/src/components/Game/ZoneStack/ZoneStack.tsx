import { useDroppable } from '@dnd-kit/core';

import { useGameAccess, useScryfallCard } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Data } from '@app/types';
import { cx } from '@app/utils';

import './ZoneStack.css';

export interface ZoneStackProps {
  gameId: number;
  playerId: number;
  zoneName: string;
  label: string;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onClick?: (zoneName: string) => void;
  onContextMenu?: (zoneName: string, event: React.MouseEvent) => void;
}

function ZoneStack({
  gameId,
  playerId,
  zoneName,
  label,
  onCardHover,
  onClick,
  onContextMenu,
}: ZoneStackProps) {
  const zone = useAppSelector((state) =>
    GameSelectors.getZone(state, gameId, playerId, zoneName),
  );
  const topCard: Data.ServerInfo_Card | undefined = zone
    ? zone.byId[zone.order[zone.order.length - 1]]
    : undefined;

  const { smallUrl } = useScryfallCard(topCard ?? null);
  const count = zone?.cardCount ?? 0;

  // Disable drops onto zones the local user can't act on (opponent zones
  // for non-judges, etc.). Server rejects the same moves; this keeps the
  // dnd-kit over-feedback honest.
  const { canAct } = useGameAccess(gameId, playerId);
  const { setNodeRef, isOver } = useDroppable({
    id: `zone-${playerId}-${zoneName}`,
    data: { targetPlayerId: playerId, targetZone: zoneName },
    disabled: !canAct,
  });

  return (
    <div
      ref={setNodeRef}
      className={cx('zone-stack', { 'zone-stack--drop-over': isOver })}
      data-testid={`zone-stack-${zoneName}`}
      onMouseEnter={() => topCard && onCardHover?.(topCard)}
      onClick={() => onClick?.(zoneName)}
      onContextMenu={(e) => onContextMenu?.(zoneName, e)}
      onKeyDown={(e) => {
        if (onClick && (e.key === 'Enter' || e.key === ' ')) {
          e.preventDefault();
          onClick(zoneName);
        }
      }}
      role={onClick ? 'button' : undefined}
      tabIndex={onClick ? 0 : undefined}
    >
      <div className="zone-stack__thumb">
        {topCard && smallUrl && !topCard.faceDown ? (
          <img className="zone-stack__image" src={smallUrl} alt={topCard.name} />
        ) : (
          <div className="zone-stack__placeholder" />
        )}
        <div className="zone-stack__count">{count}</div>
      </div>
      <div className="zone-stack__label">{label}</div>
    </div>
  );
}

export default ZoneStack;
