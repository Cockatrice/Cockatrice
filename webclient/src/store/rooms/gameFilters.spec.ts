import {
  DEFAULT_GAME_FILTERS,
  GameFilterContext,
  gameMatchesFilters,
  isGameFiltersAtDefaults,
} from './gameFilters';
import { makeGame, makeUser } from './__mocks__/rooms-fixtures';

const ctx = (overrides: Partial<GameFilterContext> = {}): GameFilterContext => ({
  isOwnUserRegistered: true,
  isUserBuddy: () => false,
  isUserIgnored: () => false,
  nowSeconds: 1_000_000,
  ...overrides,
});

describe('isGameFiltersAtDefaults', () => {
  it('returns true for the default filter state', () => {
    expect(isGameFiltersAtDefaults(DEFAULT_GAME_FILTERS)).toBe(true);
  });

  it('returns false when any boolean toggle is set', () => {
    expect(isGameFiltersAtDefaults({ ...DEFAULT_GAME_FILTERS, hideFullGames: true })).toBe(false);
  });

  it('returns false when a string filter is non-empty', () => {
    expect(isGameFiltersAtDefaults({ ...DEFAULT_GAME_FILTERS, gameNameFilter: 'foo' })).toBe(false);
  });

  it('returns false when game-type filter is non-empty', () => {
    expect(isGameFiltersAtDefaults({ ...DEFAULT_GAME_FILTERS, gameTypeFilter: [3] })).toBe(false);
  });

  it('returns false when max-age is set', () => {
    expect(isGameFiltersAtDefaults({ ...DEFAULT_GAME_FILTERS, maxGameAgeSeconds: 600 })).toBe(false);
  });
});

describe('gameMatchesFilters', () => {
  // Note: maxPlayers must be in [DEFAULT_MAX_PLAYERS_MIN, DEFAULT_MAX_PLAYERS_MAX] = [1, 99]
  // for the default range filter to accept the game. Tests set maxPlayers: 4 by convention.

  it('accepts every game when filters are at defaults', () => {
    const game = makeGame({ maxPlayers: 4, description: 'anything' });
    expect(gameMatchesFilters(game, DEFAULT_GAME_FILTERS, ctx())).toBe(true);
  });

  it('hideBuddiesOnlyGames hides games with onlyBuddies set', () => {
    const game = makeGame({ onlyBuddies: true });
    const filters = { ...DEFAULT_GAME_FILTERS, hideBuddiesOnlyGames: true };
    expect(gameMatchesFilters(game, filters, ctx())).toBe(false);
  });

  it('hideFullGames hides games at capacity', () => {
    const game = makeGame({ playerCount: 4, maxPlayers: 4 });
    const filters = { ...DEFAULT_GAME_FILTERS, hideFullGames: true };
    expect(gameMatchesFilters(game, filters, ctx())).toBe(false);
  });

  it('hideFullGames keeps games with open seats', () => {
    const game = makeGame({ playerCount: 1, maxPlayers: 4 });
    const filters = { ...DEFAULT_GAME_FILTERS, hideFullGames: true };
    expect(gameMatchesFilters(game, filters, ctx())).toBe(true);
  });

  it('hideGamesThatStarted hides games where started is true', () => {
    const game = makeGame({ started: true });
    const filters = { ...DEFAULT_GAME_FILTERS, hideGamesThatStarted: true };
    expect(gameMatchesFilters(game, filters, ctx())).toBe(false);
  });

  it('onlyRegistered hides registration-required games when own user is unregistered', () => {
    const game = makeGame({ onlyRegistered: true });
    expect(gameMatchesFilters(game, DEFAULT_GAME_FILTERS, ctx({ isOwnUserRegistered: false }))).toBe(false);
  });

  it('onlyRegistered does not affect own-registered users', () => {
    const game = makeGame({ maxPlayers: 4, onlyRegistered: true });
    expect(gameMatchesFilters(game, DEFAULT_GAME_FILTERS, ctx({ isOwnUserRegistered: true }))).toBe(true);
  });

  it('hidePasswordProtectedGames hides games with withPassword set', () => {
    const game = makeGame({ withPassword: true });
    const filters = { ...DEFAULT_GAME_FILTERS, hidePasswordProtectedGames: true };
    expect(gameMatchesFilters(game, filters, ctx())).toBe(false);
  });

  it('gameNameFilter matches case-insensitive substring against description', () => {
    const game = makeGame({ maxPlayers: 4, description: 'Modern Casual' });
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, gameNameFilter: 'modern' }, ctx())).toBe(true);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, gameNameFilter: 'legacy' }, ctx())).toBe(false);
  });

  it('creatorNameFilters matches if any substring is found in creator name', () => {
    const game = makeGame({ maxPlayers: 4, creatorInfo: makeUser({ name: 'Alice' }) });
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, creatorNameFilters: ['ali'] }, ctx())).toBe(true);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, creatorNameFilters: ['bob'] }, ctx())).toBe(false);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, creatorNameFilters: ['bob', 'ali'] }, ctx())).toBe(true);
  });

  it('gameTypeFilter requires intersection with at least one allowed type', () => {
    const game = makeGame({ maxPlayers: 4, gameTypes: [1, 2] });
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, gameTypeFilter: [3] }, ctx())).toBe(false);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, gameTypeFilter: [2, 3] }, ctx())).toBe(true);
  });

  it('maxPlayersFilter respects min and max bounds', () => {
    const game = makeGame({ maxPlayers: 4 });
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, maxPlayersFilterMin: 6 }, ctx())).toBe(false);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, maxPlayersFilterMax: 2 }, ctx())).toBe(false);
    expect(gameMatchesFilters(game, { ...DEFAULT_GAME_FILTERS, maxPlayersFilterMin: 4, maxPlayersFilterMax: 4 }, ctx())).toBe(true);
  });

  it('maxGameAgeSeconds hides games older than the cutoff', () => {
    const game = makeGame({ maxPlayers: 4, startTime: 100 });
    const filters = { ...DEFAULT_GAME_FILTERS, maxGameAgeSeconds: 60 };
    expect(gameMatchesFilters(game, filters, ctx({ nowSeconds: 200 }))).toBe(false);
    expect(gameMatchesFilters(game, filters, ctx({ nowSeconds: 150 }))).toBe(true);
  });

  it('showOnlyIfSpectatorsCanWatch requires spectatorsAllowed', () => {
    const filters = { ...DEFAULT_GAME_FILTERS, showOnlyIfSpectatorsCanWatch: true };
    expect(gameMatchesFilters(makeGame({ maxPlayers: 4, spectatorsAllowed: false }), filters, ctx())).toBe(false);
    expect(gameMatchesFilters(makeGame({ maxPlayers: 4, spectatorsAllowed: true }), filters, ctx())).toBe(true);
  });

  it('showOnlyIfSpectatorsCanChat further requires spectatorsCanChat', () => {
    const filters = {
      ...DEFAULT_GAME_FILTERS,
      showOnlyIfSpectatorsCanWatch: true,
      showOnlyIfSpectatorsCanChat: true,
    };
    expect(gameMatchesFilters(makeGame({ maxPlayers: 4, spectatorsAllowed: true, spectatorsCanChat: false }), filters, ctx())).toBe(false);
    expect(gameMatchesFilters(makeGame({ maxPlayers: 4, spectatorsAllowed: true, spectatorsCanChat: true }), filters, ctx())).toBe(true);
  });

  it('hideIgnoredUserGames consults the isUserIgnored predicate', () => {
    const game = makeGame({ maxPlayers: 4, creatorInfo: makeUser({ name: 'troll' }) });
    const filters = { ...DEFAULT_GAME_FILTERS, hideIgnoredUserGames: true };
    expect(gameMatchesFilters(game, filters, ctx({ isUserIgnored: (n) => n === 'troll' }))).toBe(false);
    expect(gameMatchesFilters(game, filters, ctx({ isUserIgnored: () => false }))).toBe(true);
  });
});
