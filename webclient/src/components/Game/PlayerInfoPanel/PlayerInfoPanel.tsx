import { cx } from '@app/utils';
import type { Data } from '@app/types';

import { cssColor, usePlayerInfoPanel } from './usePlayerInfoPanel';

import './PlayerInfoPanel.css';

export interface PlayerInfoPanelProps {
  gameId: number;
  playerId: number;
  canEdit?: boolean;
  onRequestCreateCounter?: () => void;
  onContextMenu?: (event: React.MouseEvent) => void;
}

function PlayerInfoPanel({
  gameId,
  playerId,
  canEdit = false,
  onRequestCreateCounter,
  onContextMenu,
}: PlayerInfoPanelProps) {
  const {
    player,
    isHost,
    lifeCounter,
    otherCounters,
    editingId,
    editDraft,
    setEditDraft,
    beginEdit,
    commitEdit,
    cancelEdit,
    handleIncrement,
    handleDelete,
  } = usePlayerInfoPanel({ gameId, playerId });

  if (!player) {
    return <div className="player-info-panel player-info-panel--empty" />;
  }

  const name = player.properties.userInfo?.name ?? '(unknown)';
  const ping = player.properties.pingSeconds ?? 0;
  const conceded = player.properties.conceded;
  const ready = player.properties.readyStart;
  const sideboardLocked = player.properties.sideboardLocked ?? false;

  const renderCounterRow = (c: Data.ServerInfo_Counter) => (
    <li
      key={c.id}
      className="player-info-panel__counter"
      data-testid={`counter-${c.id}`}
    >
      <span
        className="player-info-panel__swatch"
        style={{ background: cssColor(c.counterColor) }}
      />
      <span className="player-info-panel__counter-name" title={c.name}>{c.name}</span>
      {canEdit && (
        <button
          type="button"
          className="player-info-panel__counter-btn"
          aria-label={`decrement ${c.name}`}
          onClick={() => handleIncrement(c.id, -1)}
        >
          −
        </button>
      )}
      {editingId === c.id ? (
        <input
          type="number"
          autoFocus
          className="player-info-panel__counter-input"
          value={editDraft}
          onChange={(e) => setEditDraft(e.target.value)}
          onBlur={() => commitEdit(c.id)}
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              commitEdit(c.id);
            }
            if (e.key === 'Escape') {
              cancelEdit();
            }
          }}
          aria-label={`set ${c.name}`}
        />
      ) : (
        <span
          className={cx('player-info-panel__counter-value', {
            'player-info-panel__counter-value--editable': canEdit,
          })}
          onClick={canEdit ? () => beginEdit(c.id, c.count) : undefined}
          role={canEdit ? 'button' : undefined}
          tabIndex={canEdit ? 0 : undefined}
        >
          {c.count}
        </span>
      )}
      {canEdit && (
        <button
          type="button"
          className="player-info-panel__counter-btn"
          aria-label={`increment ${c.name}`}
          onClick={() => handleIncrement(c.id, +1)}
        >
          +
        </button>
      )}
      {canEdit && (
        <button
          type="button"
          className="player-info-panel__counter-btn player-info-panel__counter-btn--del"
          aria-label={`delete ${c.name}`}
          onClick={() => handleDelete(c.id)}
        >
          ×
        </button>
      )}
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
        {sideboardLocked && (
          <span
            className="player-info-panel__sideboard-lock"
            aria-label="sideboard locked"
            title="Sideboard locked"
          >
            🔒
          </span>
        )}
        <span className="player-info-panel__ping" title={`ping ${ping}s`}>
          {ping}s
        </span>
      </div>

      {conceded && <div className="player-info-panel__flag">Conceded</div>}
      {!conceded && ready && <div className="player-info-panel__flag player-info-panel__flag--ready">Ready</div>}

      {lifeCounter && (
        <div
          className="player-info-panel__life"
          data-testid={`life-${playerId}`}
          style={{ borderColor: cssColor(lifeCounter.counterColor) }}
        >
          {canEdit && (
            <button
              type="button"
              className="player-info-panel__life-btn"
              aria-label="decrement Life"
              onClick={() => handleIncrement(lifeCounter.id, -1)}
            >
              −
            </button>
          )}
          {editingId === lifeCounter.id ? (
            <input
              type="number"
              autoFocus
              className="player-info-panel__life-input"
              value={editDraft}
              onChange={(e) => setEditDraft(e.target.value)}
              onBlur={() => commitEdit(lifeCounter.id)}
              onKeyDown={(e) => {
                if (e.key === 'Enter') {
                  commitEdit(lifeCounter.id);
                }
                if (e.key === 'Escape') {
                  cancelEdit();
                }
              }}
              aria-label="set Life"
            />
          ) : (
            <span
              className={cx('player-info-panel__life-value', {
                'player-info-panel__life-value--editable': canEdit,
              })}
              onClick={canEdit ? () => beginEdit(lifeCounter.id, lifeCounter.count) : undefined}
              role={canEdit ? 'button' : undefined}
              tabIndex={canEdit ? 0 : undefined}
              aria-label={`Life: ${lifeCounter.count}`}
            >
              {lifeCounter.count}
            </span>
          )}
          {canEdit && (
            <button
              type="button"
              className="player-info-panel__life-btn"
              aria-label="increment Life"
              onClick={() => handleIncrement(lifeCounter.id, +1)}
            >
              +
            </button>
          )}
          <div className="player-info-panel__life-label">LIFE</div>
        </div>
      )}

      <ul className="player-info-panel__counters">
        {otherCounters.length === 0 && !lifeCounter && (
          <li className="player-info-panel__counter player-info-panel__counter--empty">
            no counters
          </li>
        )}
        {otherCounters.map(renderCounterRow)}
      </ul>

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
