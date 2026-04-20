import { useEffect, useRef, useState } from 'react';
import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Enriched } from '@app/types';

import './GameLog.css';

const EMPTY_MESSAGES: Enriched.GameMessage[] = [];

function formatElapsed(totalSeconds: number): string {
  const s = Math.max(0, Math.floor(totalSeconds));
  const hh = String(Math.floor(s / 3600)).padStart(2, '0');
  const mm = String(Math.floor((s % 3600) / 60)).padStart(2, '0');
  const ss = String(s % 60).padStart(2, '0');
  return `${hh}:${mm}:${ss}`;
}

export interface GameLogProps {
  gameId: number | undefined;
}

function GameLog({ gameId }: GameLogProps) {
  const webClient = useWebClient();
  const messages = useAppSelector((state) =>
    gameId != null ? GameSelectors.getMessages(state, gameId) : EMPTY_MESSAGES,
  );
  const players = useAppSelector((state) =>
    gameId != null ? GameSelectors.getPlayers(state, gameId) : undefined,
  );
  const secondsElapsed = useAppSelector((state) =>
    gameId != null ? GameSelectors.getSecondsElapsed(state, gameId) : 0,
  );

  // Local 1Hz ticker, resynced from Redux whenever a server event delivers a
  // fresh `secondsElapsed`. Mirrors desktop's QTimer(1000) +
  // setGameTime(event.seconds_elapsed()) pattern in game_state.cpp.
  const [displaySeconds, setDisplaySeconds] = useState(secondsElapsed);

  useEffect(() => {
    setDisplaySeconds(secondsElapsed);
  }, [secondsElapsed]);

  useEffect(() => {
    if (gameId == null) {
      return undefined;
    }
    const id = window.setInterval(() => {
      setDisplaySeconds((prev) => prev + 1);
    }, 1000);
    return () => window.clearInterval(id);
  }, [gameId]);

  const [draft, setDraft] = useState('');

  const listRef = useRef<HTMLDivElement>(null);
  // Desktop pins the log to the bottom unless the user has scrolled up to read backlog.
  // Capture pin state before the new line renders so auto-scroll only fires when the
  // user was already following the tail.
  const wasPinnedRef = useRef(true);
  useEffect(() => {
    const el = listRef.current;
    if (!el) {
      return;
    }
    if (wasPinnedRef.current) {
      el.scrollTop = el.scrollHeight;
    }
  }, [messages.length]);

  const handleMessagesScroll = () => {
    const el = listRef.current;
    if (!el) {
      return;
    }
    wasPinnedRef.current = el.scrollTop + el.clientHeight >= el.scrollHeight - 2;
  };

  const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    if (gameId == null) {
      return;
    }
    const trimmed = draft.trim();
    if (trimmed.length === 0) {
      return;
    }
    webClient.request.game.gameSay(gameId, { message: trimmed });
    setDraft('');
  };

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
