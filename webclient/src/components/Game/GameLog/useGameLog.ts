import { useEffect, useRef, useState, RefObject } from 'react';
import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Enriched } from '@app/types';

const EMPTY_MESSAGES: Enriched.GameMessage[] = [];

export function formatElapsed(totalSeconds: number): string {
  const s = Math.max(0, Math.floor(totalSeconds));
  const hh = String(Math.floor(s / 3600)).padStart(2, '0');
  const mm = String(Math.floor((s % 3600) / 60)).padStart(2, '0');
  const ss = String(s % 60).padStart(2, '0');
  return `${hh}:${mm}:${ss}`;
}

export interface GameLog {
  messages: Enriched.GameMessage[];
  players: Record<number, Enriched.PlayerEntry> | undefined;
  displaySeconds: number;
  draft: string;
  setDraft: (v: string) => void;
  handleMessagesScroll: () => void;
  handleSubmit: (e: React.FormEvent<HTMLFormElement>) => void;
}

export interface UseGameLogArgs {
  gameId: number | undefined;
  listRef: RefObject<HTMLDivElement | null>;
}

export function useGameLog({ gameId, listRef }: UseGameLogArgs): GameLog {
  const webClient = useWebClient();
  // getMessages falls back to a shared EMPTY_ARRAY typed as ServerInfo_Card[]
  // (see game.selectors.ts). The runtime array is empty, so the cast is safe;
  // fixing the selector's fallback type is out of scope for this refactor.
  const messages = useAppSelector((state) =>
    gameId != null ? GameSelectors.getMessages(state, gameId) : EMPTY_MESSAGES,
  ) as Enriched.GameMessage[];
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
  }, [messages.length, listRef]);

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

  return {
    messages,
    players,
    displaySeconds,
    draft,
    setDraft,
    handleMessagesScroll,
    handleSubmit,
  };
}
