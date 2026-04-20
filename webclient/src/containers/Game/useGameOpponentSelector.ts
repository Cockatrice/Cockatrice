import { useEffect, useMemo, useState } from 'react';

import type { Enriched } from '@app/types';

export interface OpponentEntry {
  playerId: number;
  name: string;
}

export interface GameOpponentSelector {
  opponents: OpponentEntry[];
  selectedOpponentId: number | undefined;
  setSelectedOpponentId: (id: number | undefined) => void;
  shownOpponentId: number | undefined;
  revealPlayers: OpponentEntry[];
}

export function useGameOpponentSelector(
  game: Enriched.GameEntry | undefined,
): GameOpponentSelector {
  const [selectedOpponentId, setSelectedOpponentId] = useState<number | undefined>();

  const opponents = useMemo<OpponentEntry[]>(() => {
    if (!game) {
      return [];
    }
    return Object.values(game.players)
      .filter((p) => p.properties.playerId !== game.localPlayerId)
      .map((p) => ({
        playerId: p.properties.playerId,
        name: p.properties.userInfo?.name ?? `p${p.properties.playerId}`,
      }));
  }, [game]);

  useEffect(() => {
    if (selectedOpponentId == null && opponents.length > 0) {
      setSelectedOpponentId(opponents[0].playerId);
    }
    if (selectedOpponentId != null && !opponents.some((o) => o.playerId === selectedOpponentId)) {
      setSelectedOpponentId(opponents[0]?.playerId);
    }
  }, [opponents, selectedOpponentId]);

  const shownOpponentId = selectedOpponentId ?? opponents[0]?.playerId;

  return {
    opponents,
    selectedOpponentId,
    setSelectedOpponentId,
    shownOpponentId,
    revealPlayers: opponents,
  };
}
