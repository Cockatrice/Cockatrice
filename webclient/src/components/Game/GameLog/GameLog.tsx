import { useRef } from 'react';

import { formatElapsed, useGameLog } from './useGameLog';

import './GameLog.css';

export interface GameLogProps {
  gameId: number | undefined;
}

function GameLog({ gameId }: GameLogProps) {
  const listRef = useRef<HTMLDivElement>(null);
  const {
    messages,
    players,
    displaySeconds,
    draft,
    setDraft,
    handleMessagesScroll,
    handleSubmit,
  } = useGameLog({ gameId, listRef });

  return (
    <div className="game-log" data-testid="game-log">
      <div className="game-log__heading">Log</div>
      {gameId != null && (
        <div className="game-log__timer" data-testid="game-log-timer">
          {formatElapsed(displaySeconds)}
        </div>
      )}
      <div className="game-log__messages" ref={listRef} onScroll={handleMessagesScroll}>
        {messages.length === 0 && (
          <div className="game-log__empty">no messages</div>
        )}
        {messages.map((m, idx) => {
          const isEvent = m.kind === 'event';
          const name = players?.[m.playerId]?.properties.userInfo?.name ?? `p${m.playerId}`;
          const lineClass = isEvent ? 'game-log__line game-log__line--event' : 'game-log__line';
          return (
            <div key={idx} className={lineClass}>
              {!isEvent && <span className="game-log__author">{name}:</span>}
              <span className="game-log__text">{m.message}</span>
            </div>
          );
        })}
      </div>
      <form className="game-log__input-row" onSubmit={handleSubmit}>
        <label className="game-log__input-label" htmlFor="game-log-say-input">
          Say:
        </label>
        <input
          id="game-log-say-input"
          type="text"
          className="game-log__input"
          value={draft}
          onChange={(e) => setDraft(e.target.value)}
          disabled={gameId == null}
          aria-label="game chat input"
        />
      </form>
    </div>
  );
}

export default GameLog;
