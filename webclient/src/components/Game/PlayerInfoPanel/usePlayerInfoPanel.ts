import { useState } from 'react';
import { useWebClient } from '@app/hooks';
import { GameSelectors, useAppSelector } from '@app/store';
import type { Data, Enriched } from '@app/types';

export function cssColor(c: { r: number; g: number; b: number; a: number } | undefined): string {
  if (!c) {
    return '#666';
  }
  return `rgba(${c.r}, ${c.g}, ${c.b}, ${(c.a ?? 255) / 255})`;
}

// Desktop renders Life larger/bolder than other counters (see
// cockatrice/src/game/player/player.cpp PlayerTarget sizing). We special-
// case the counter whose name is exactly 'Life' (case-insensitive) and
// pull it out of the regular counter list into a prominent life block.
export function isLifeCounter(c: { name: string }): boolean {
  return c.name.trim().toLowerCase() === 'life';
}

export interface PlayerInfoPanel {
  player: Enriched.PlayerEntry | undefined;
  isHost: boolean;
  lifeCounter: Data.ServerInfo_Counter | undefined;
  otherCounters: Data.ServerInfo_Counter[];
  editingId: number | null;
  editDraft: string;
  setEditDraft: (v: string) => void;
  beginEdit: (counterId: number, currentValue: number) => void;
  commitEdit: (counterId: number) => void;
  cancelEdit: () => void;
  handleIncrement: (counterId: number, delta: number) => void;
  handleDelete: (counterId: number) => void;
}

export interface UsePlayerInfoPanelArgs {
  gameId: number;
  playerId: number;
}

export function usePlayerInfoPanel({
  gameId,
  playerId,
}: UsePlayerInfoPanelArgs): PlayerInfoPanel {
  const webClient = useWebClient();
  const player = useAppSelector((state) => GameSelectors.getPlayer(state, gameId, playerId));
  const counters = useAppSelector((state) => GameSelectors.getCounters(state, gameId, playerId));
  const hostId = useAppSelector((state) => GameSelectors.getHostId(state, gameId));

  const [editingId, setEditingId] = useState<number | null>(null);
  const [editDraft, setEditDraft] = useState('');

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

  return {
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
  };
}
