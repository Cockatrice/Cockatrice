import { Data, Enriched } from '@app/types';
import { GameFilters } from './rooms.interfaces';

export const DEFAULT_MAX_PLAYERS_MIN = 1;
export const DEFAULT_MAX_PLAYERS_MAX = 99;
export const DEFAULT_MAX_GAME_AGE_SECONDS = 0;

export const DEFAULT_GAME_FILTERS: GameFilters = {
  hideBuddiesOnlyGames: false,
  hideIgnoredUserGames: false,
  hideFullGames: false,
  hideGamesThatStarted: false,
  hidePasswordProtectedGames: false,
  hideNotBuddyCreatedGames: false,
  hideOpenDecklistGames: false,
  gameNameFilter: '',
  creatorNameFilters: [],
  gameTypeFilter: [],
  maxPlayersFilterMin: DEFAULT_MAX_PLAYERS_MIN,
  maxPlayersFilterMax: DEFAULT_MAX_PLAYERS_MAX,
  maxGameAgeSeconds: DEFAULT_MAX_GAME_AGE_SECONDS,
  showOnlyIfSpectatorsCanWatch: false,
  showSpectatorPasswordProtected: false,
  showOnlyIfSpectatorsCanChat: false,
  showOnlyIfSpectatorsCanSeeHands: false,
};

export function isGameFiltersAtDefaults(filters: GameFilters): boolean {
  return (
    !filters.hideBuddiesOnlyGames &&
    !filters.hideIgnoredUserGames &&
    !filters.hideFullGames &&
    !filters.hideGamesThatStarted &&
    !filters.hidePasswordProtectedGames &&
    !filters.hideNotBuddyCreatedGames &&
    !filters.hideOpenDecklistGames &&
    filters.gameNameFilter === '' &&
    filters.creatorNameFilters.length === 0 &&
    filters.gameTypeFilter.length === 0 &&
    filters.maxPlayersFilterMin === DEFAULT_MAX_PLAYERS_MIN &&
    filters.maxPlayersFilterMax === DEFAULT_MAX_PLAYERS_MAX &&
    filters.maxGameAgeSeconds === DEFAULT_MAX_GAME_AGE_SECONDS &&
    !filters.showOnlyIfSpectatorsCanWatch &&
    !filters.showSpectatorPasswordProtected &&
    !filters.showOnlyIfSpectatorsCanChat &&
    !filters.showOnlyIfSpectatorsCanSeeHands
  );
}

export interface GameFilterContext {
  isOwnUserRegistered: boolean;
  isUserBuddy: (name: string) => boolean;
  isUserIgnored: (name: string) => boolean;
  nowSeconds: number;
}

/**
 * Mirrors GamesProxyModel::filterAcceptsRow
 * (cockatrice/src/interface/widgets/server/games_model.cpp).
 */
export function gameMatchesFilters(
  game: Enriched.Game,
  filters: GameFilters,
  ctx: GameFilterContext,
): boolean {
  const info: Data.ServerInfo_Game = game.info;
  const creatorName = info.creatorInfo?.name ?? '';

  if (filters.hideBuddiesOnlyGames && info.onlyBuddies) {
    return false;
  }
  if (filters.hideOpenDecklistGames && info.shareDecklistsOnLoad) {
    return false;
  }
  if (filters.hideIgnoredUserGames && ctx.isUserIgnored(creatorName)) {
    return false;
  }
  if (filters.hideNotBuddyCreatedGames && !ctx.isUserBuddy(creatorName)) {
    return false;
  }
  if (filters.hideFullGames && info.playerCount === info.maxPlayers) {
    return false;
  }
  if (filters.hideGamesThatStarted && info.started) {
    return false;
  }
  if (!ctx.isOwnUserRegistered && info.onlyRegistered) {
    return false;
  }
  if (filters.hidePasswordProtectedGames && info.withPassword) {
    return false;
  }
  if (filters.gameNameFilter !== '') {
    if (!info.description.toLowerCase().includes(filters.gameNameFilter.toLowerCase())) {
      return false;
    }
  }
  if (filters.creatorNameFilters.length > 0) {
    const lowerCreator = creatorName.toLowerCase();
    const found = filters.creatorNameFilters.some((substr) =>
      lowerCreator.includes(substr.toLowerCase()),
    );
    if (!found) {
      return false;
    }
  }
  if (filters.gameTypeFilter.length > 0) {
    const allowed = new Set(filters.gameTypeFilter);
    const intersects = info.gameTypes.some((id) => allowed.has(id));
    if (!intersects) {
      return false;
    }
  }
  if (info.maxPlayers < filters.maxPlayersFilterMin) {
    return false;
  }
  if (info.maxPlayers > filters.maxPlayersFilterMax) {
    return false;
  }
  if (filters.maxGameAgeSeconds > 0) {
    const ageSeconds = ctx.nowSeconds - info.startTime;
    if (ageSeconds > filters.maxGameAgeSeconds) {
      return false;
    }
  }
  if (filters.showOnlyIfSpectatorsCanWatch) {
    if (!info.spectatorsAllowed) {
      return false;
    }
    if (!filters.showSpectatorPasswordProtected && info.spectatorsNeedPassword) {
      return false;
    }
    if (filters.showOnlyIfSpectatorsCanChat && !info.spectatorsCanChat) {
      return false;
    }
    if (filters.showOnlyIfSpectatorsCanSeeHands && !info.spectatorsOmniscient) {
      return false;
    }
  }
  return true;
}
