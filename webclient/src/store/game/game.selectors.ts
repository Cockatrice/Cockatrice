import { GamesState, GameEntry, PlayerEntry, ZoneEntry } from './game.interfaces';

interface State {
  games: GamesState;
}

export const Selectors = {
  getGames: ({ games }: State): { [gameId: number]: GameEntry } => games.games,

  getGame: ({ games }: State, gameId: number): GameEntry | undefined => games.games[gameId],

  getPlayers: ({ games }: State, gameId: number): { [playerId: number]: PlayerEntry } | undefined =>
    games.games[gameId]?.players,

  getPlayer: ({ games }: State, gameId: number, playerId: number): PlayerEntry | undefined =>
    games.games[gameId]?.players[playerId],

  getLocalPlayerId: ({ games }: State, gameId: number): number | undefined =>
    games.games[gameId]?.localPlayerId,

  getLocalPlayer: (state: State, gameId: number): PlayerEntry | undefined => {
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
  ): { [zoneName: string]: ZoneEntry } | undefined =>
    games.games[gameId]?.players[playerId]?.zones,

  getZone: (
    { games }: State,
    gameId: number,
    playerId: number,
    zoneName: string
  ): ZoneEntry | undefined => games.games[gameId]?.players[playerId]?.zones[zoneName],

  getCards: ({ games }: State, gameId: number, playerId: number, zoneName: string) =>
    games.games[gameId]?.players[playerId]?.zones[zoneName]?.cards ?? [],

  getCounters: ({ games }: State, gameId: number, playerId: number) =>
    games.games[gameId]?.players[playerId]?.counters ?? {},

  getArrows: ({ games }: State, gameId: number, playerId: number) =>
    games.games[gameId]?.players[playerId]?.arrows ?? {},

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
    games.games[gameId]?.messages ?? [],

  getActiveGameIds: ({ games }: State): number[] =>
    Object.keys(games.games).map(Number),
};
