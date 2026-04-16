import { createSelector } from '@reduxjs/toolkit';
import type { Data, Enriched } from '@app/types';
import { GamesState } from './game.interfaces';

interface State {
  games: GamesState;
}

const EMPTY_ARRAY: Data.ServerInfo_Card[] = [];
const EMPTY_OBJECT = {} as Record<string, never>;

/**
 * Memoized cache for materialized zone card arrays. Keyed by the zone object
 * identity so that repeated selector calls on the same zone reuse the same
 * array reference — this preserves React referential equality and avoids
 * spurious re-renders when `getCards` is called from a selector.
 */
const zoneCardsCache = new WeakMap<Enriched.ZoneEntry, Data.ServerInfo_Card[]>();

function materializeZoneCards(zone: Enriched.ZoneEntry): Data.ServerInfo_Card[] {
  const cached = zoneCardsCache.get(zone);
  if (cached) {
    return cached;
  }
  const arr = zone.order.map(id => zone.byId[id]);
  zoneCardsCache.set(zone, arr);
  return arr;
}

export const Selectors = {
  getGames: ({ games }: State): { [gameId: number]: Enriched.GameEntry } => games.games,

  getGame: ({ games }: State, gameId: number): Enriched.GameEntry | undefined => games.games[gameId],

  getPlayers: ({ games }: State, gameId: number): { [playerId: number]: Enriched.PlayerEntry } | undefined =>
    games.games[gameId]?.players,

  getPlayer: ({ games }: State, gameId: number, playerId: number): Enriched.PlayerEntry | undefined =>
    games.games[gameId]?.players[playerId],

  getLocalPlayerId: ({ games }: State, gameId: number): number | undefined =>
    games.games[gameId]?.localPlayerId,

  getLocalPlayer: (state: State, gameId: number): Enriched.PlayerEntry | undefined => {
    const game = state.games.games[gameId];
    if (!game) {
      return undefined;
    }
    return game.players[game.localPlayerId];
  },

  getZones: (
    { games }: State,
    gameId: number,
    playerId: number
  ): { [zoneName: string]: Enriched.ZoneEntry } | undefined =>
    games.games[gameId]?.players[playerId]?.zones,

  getZone: (
    { games }: State,
    gameId: number,
    playerId: number,
    zoneName: string
  ): Enriched.ZoneEntry | undefined => games.games[gameId]?.players[playerId]?.zones[zoneName],

  getCards: ({ games }: State, gameId: number, playerId: number, zoneName: string): Data.ServerInfo_Card[] => {
    const zone = games.games[gameId]?.players[playerId]?.zones[zoneName];
    return zone ? materializeZoneCards(zone) : EMPTY_ARRAY;
  },

  getCounters: ({ games }: State, gameId: number, playerId: number) =>
    games.games[gameId]?.players[playerId]?.counters ?? EMPTY_OBJECT,

  getArrows: ({ games }: State, gameId: number, playerId: number) =>
    games.games[gameId]?.players[playerId]?.arrows ?? EMPTY_OBJECT,

  getActivePlayerId: ({ games }: State, gameId: number): number | undefined =>
    games.games[gameId]?.activePlayerId,

  getActivePhase: ({ games }: State, gameId: number): number | undefined =>
    games.games[gameId]?.activePhase,

  isStarted: ({ games }: State, gameId: number): boolean =>
    games.games[gameId]?.started ?? false,

  isSpectator: ({ games }: State, gameId: number): boolean =>
    games.games[gameId]?.spectator ?? false,

  isReversed: ({ games }: State, gameId: number): boolean =>
    games.games[gameId]?.reversed ?? false,

  getMessages: ({ games }: State, gameId: number) =>
    games.games[gameId]?.messages ?? EMPTY_ARRAY,

  getActiveGameIds: createSelector(
    [({ games }: State) => games.games],
    (games) => Object.keys(games).map(Number)
  ),
};
