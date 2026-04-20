import { useMemo } from 'react';
import { App, Data } from '@app/types';
import { GameSelectors, useAppSelector } from '@app/store';
import { useSettings } from '@app/hooks';

export interface Battlefield {
  rows: Data.ServerInfo_Card[][];
  rowOrder: number[];
  isInverted: boolean;
}

export interface UseBattlefieldArgs {
  gameId: number;
  playerId: number;
  mirrored: boolean;
}

const ROW_COUNT = 3;

function rowIndexFor(card: Data.ServerInfo_Card): number {
  const y = card.y ?? 0;
  return Math.max(0, Math.min(ROW_COUNT - 1, y));
}

export function useBattlefield({ gameId, playerId, mirrored }: UseBattlefieldArgs): Battlefield {
  const cards = useAppSelector((state) =>
    GameSelectors.getCards(state, gameId, playerId, App.ZoneName.TABLE),
  );

  const { value: settings } = useSettings();
  const invertVerticalCoordinate = settings?.invertVerticalCoordinate ?? false;
  // Mirrors desktop TableZone::isInverted() — XOR of per-player mirrored and
  // the global invertVerticalCoordinate preference.
  const isInverted = mirrored !== invertVerticalCoordinate;

  const rows = useMemo<Data.ServerInfo_Card[][]>(() => {
    const bucketed: Data.ServerInfo_Card[][] = Array.from({ length: ROW_COUNT }, () => []);
    for (const card of cards) {
      bucketed[rowIndexFor(card)].push(card);
    }
    for (const row of bucketed) {
      row.sort((a, b) => (a.x ?? 0) - (b.x ?? 0));
    }
    return bucketed;
  }, [cards]);

  const rowOrder = isInverted ? [2, 1, 0] : [0, 1, 2];

  return { rows, rowOrder, isInverted };
}
