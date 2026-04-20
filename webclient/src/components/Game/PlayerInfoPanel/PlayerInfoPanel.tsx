import { useState } from 'react';
import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import { cx } from '@app/utils';
import type { Data } from '@app/types';

import './PlayerInfoPanel.css';

export interface PlayerInfoPanelProps {
  gameId: number;
  playerId: number;
  canEdit?: boolean;
  onRequestCreateCounter?: () => void;
  onContextMenu?: (event: React.MouseEvent) => void;
}

function cssColor(c: { r: number; g: number; b: number; a: number } | undefined): string {
  if (!c) {
    return '#666';
  }
  return `rgba(${c.r}, ${c.g}, ${c.b}, ${(c.a ?? 255) / 255})`;
}

// Desktop renders Life larger/bolder than other counters (see
// cockatrice/src/game/player/player.cpp PlayerTarget sizing). We special-
// case the counter whose name is exactly 'Life' (case-insensitive) and
// pull it out of the regular counter list into a prominent life block.
function isLifeCounter(c: { name: string }): boolean {
  return c.name.trim().toLowerCase() === 'life';
}

function PlayerInfoPanel({
  gameId,
  playerId,
  canEdit = false,
  onRequestCreateCounter,
  onContextMenu,
}: PlayerInfoPanelProps) {
  const webClient = useWebClient();
  const player = useAppSelector((state) => GameSelectors.getPlayer(state, gameId, playerId));
  const counters = useAppSelector((state) => GameSelectors.getCounters(state, gameId, playerId));
  const hostId = useAppSelector((state) => GameSelectors.getHostId(state, gameId));

  const [editingId, setEditingId] = useState<number | null>(null);
  const [editDraft, setEditDraft] = useState('');

  if (!player) {
    return <div className="player-info-panel player-info-panel--empty" />;
  }

  const name = player.properties.userInfo?.name ?? '(unknown)';
  const ping = player.properties.pingSeconds ?? 0;
  const conceded = player.properties.conceded;
  const ready = player.properties.readyStart;
  const sideboardLocked = player.properties.sideboardLocked ?? false;
  const isHost = hostId != null && hostId === playerId;
  const allCounters = Object.values(counters);
  const lifeCounter = allCounters.find(isLifeCounter);
  const otherCounters = allCounters.filter((c) => !isLifeCounter(c));

  const handleIncrement = (counterId: number, delta: number) => {
    webClient.request.game.incCounter(gameId, { counterId, delta });
  };

  const handleDelete = (counterId: number) => {
    webClient.request.game.delCounter(gameId, { counterId });
  };

  const beginEdit = (counterId: number, currentValue: number) => {
    setEditingId(counterId);
    setEditDraft(String(currentValue));
  };

  const commitEdit = (counterId: number) => {
    const trimmed = editDraft.trim();
    // Empty input cancels the edit (desktop inline edits treat blur-with-
    // no-change and blur-with-empty-string identically). Prior behavior
    // coerced '' → 0 because `Number('')` is 0 and `Number.isInteger(0)` is
    // true, which surprised users expecting cancel-on-blank.
    if (trimmed.length === 0) {
      setEditingId(null);
      return;
    }
    const value = Number(trimmed);
    if (Number.isInteger(value)) {
      webClient.request.game.setCounter(gameId, { counterId, value });
    }
    setEditingId(null);
  };

  const cancelEdit = () => {
    setEditingId(null);
  };

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
