import { cx } from '@app/utils';
import { App, Data } from '@app/types';

import ZoneStack from '../ZoneStack/ZoneStack';

import { counterCssColor, usePlayerInfoPanel } from './usePlayerInfoPanel';

import './PlayerInfoPanel.css';

// All four zones render as landscape thumbs in the info rail. Hand sits
// between Deck and Graveyard to match desktop's hand counter placement.
const ZONE_ROWS: Array<{ name: string; label: string; rotated?: boolean }> = [
  { name: App.ZoneName.DECK, label: 'Deck', rotated: true },
  { name: App.ZoneName.HAND, label: 'Hand', rotated: true },
  { name: App.ZoneName.GRAVE, label: 'Graveyard', rotated: true },
  { name: App.ZoneName.EXILE, label: 'Exile', rotated: true },
];

export interface PlayerInfoPanelProps {
  gameId: number;
  playerId: number;
  canEdit?: boolean;
  onRequestCreateCounter?: () => void;
  onContextMenu?: (event: React.MouseEvent) => void;
  onCardHover?: (card: Data.ServerInfo_Card) => void;
  onZoneClick?: (playerId: number, zoneName: string) => void;
  onZoneContextMenu?: (playerId: number, zoneName: string, event: React.MouseEvent) => void;
}

function PlayerInfoPanel({
  gameId,
  playerId,
  canEdit = false,
  onRequestCreateCounter,
  onContextMenu,
  onCardHover,
  onZoneClick,
  onZoneContextMenu,
}: PlayerInfoPanelProps) {
  const { player, isHost, lifeCounter, otherCounters, handleIncrement } = usePlayerInfoPanel({
    gameId,
    playerId,
  });

  if (!player) {
    return <div className="player-info-panel player-info-panel--empty" />;
  }

  const name = player.properties.userInfo?.name ?? '(unknown)';
  const conceded = player.properties.conceded;
  const ready = player.properties.readyStart;

  const counterHandlers = (c: Data.ServerInfo_Counter) =>
    canEdit
      ? {
        role: 'button' as const,
        onClick: (e: React.MouseEvent) => {
          e.stopPropagation();
          handleIncrement(c.id, +1);
        },
        // stopPropagation prevents the panel's onContextMenu (player menu)
        // from firing when the user right-clicks a counter to decrement.
        onContextMenu: (e: React.MouseEvent) => {
          e.preventDefault();
          e.stopPropagation();
          handleIncrement(c.id, -1);
        },
      }
      : {};

  const renderCounterCircle = (c: Data.ServerInfo_Counter, modifier?: string) => (
    <li
      key={c.id}
      className={cx('player-info-panel__counter', modifier)}
      data-testid={`counter-${c.id}`}
      style={{ background: counterCssColor(c) }}
      title={c.name}
      aria-label={`${c.name}: ${c.count}`}
      {...counterHandlers(c)}
    >
      <span className="player-info-panel__counter-value">{c.count}</span>
    </li>
  );

  return (
    <div
      className="player-info-panel"
      data-testid={`player-info-${playerId}`}
      onContextMenu={onContextMenu}
    >
      <div className="player-info-panel__header">
        {isHost && (
          <span
            className="player-info-panel__host-badge"
            aria-label="host"
            title="Host"
          >
            ♛
          </span>
        )}
        <span className="player-info-panel__name">{name}</span>
        {lifeCounter && (
          <ul className="player-info-panel__life-slot">
            {renderCounterCircle(lifeCounter, 'player-info-panel__counter--life')}
          </ul>
        )}
      </div>

      {conceded && <div className="player-info-panel__flag">Conceded</div>}
      {!conceded && ready && <div className="player-info-panel__flag player-info-panel__flag--ready">Ready</div>}

      <div className="player-info-panel__body">
        <ul className="player-info-panel__counters">
          {otherCounters.map((c) => renderCounterCircle(c))}
        </ul>
        <div className="player-info-panel__zones">
          {ZONE_ROWS.map((z) => {
            // Hand is context-menu only: desktop's hand counter doesn't open
            // a zone view on left-click, and HandZone already renders the cards.
            const clickHandler =
              onZoneClick && z.name !== App.ZoneName.HAND
                ? (name: string) => onZoneClick(playerId, name)
                : undefined;
            return (
              <ZoneStack
                key={z.name}
                gameId={gameId}
                playerId={playerId}
                zoneName={z.name}
                label={z.label}
                rotated={z.rotated}
                onCardHover={onCardHover}
                onClick={clickHandler}
                onContextMenu={
                  onZoneContextMenu
                    ? (name, e) => onZoneContextMenu(playerId, name, e)
                    : undefined
                }
              />
            );
          })}
        </div>
      </div>

      {canEdit && onRequestCreateCounter && (
        <button
          type="button"
          className="player-info-panel__new-counter"
          onClick={onRequestCreateCounter}
        >
          + New counter
        </button>
      )}
    </div>
  );
}

export default PlayerInfoPanel;
