import { createSelector } from '@reduxjs/toolkit';
import { Data, Enriched } from '@app/types';
import { SortUtil } from '../common';
import { ServerState } from './server.interfaces';

interface State {
  server: ServerState
}

const EMPTY_USERS: Data.ServerInfo_User[] = [];
const EMPTY_REPLAYS: Data.ServerInfo_ReplayMatch[] = [];

export const Selectors = {
  getInitialized: ({ server }: State) => server.initialized,
  getMessage: ({ server }: State) => server.info.message,
  getName: ({ server }: State) => server.info.name,
  getVersion: ({ server }: State) => server.info.version,
  getDescription: ({ server }: State) => server.status.description,
  getState: ({ server }: State) => server.status.state,
  getConnectionAttemptMade: ({ server }: State) => server.status.connectionAttemptMade,
  getUser: ({ server }: State) => server.user,

  /** True when the server status has reached LOGGED_IN. */
  getIsConnected: createSelector(
    [({ server }: State) => server.status.state],
    (state): boolean => state === Enriched.StatusEnum.LOGGED_IN
  ),

  /** True when the currently logged-in user has the IsModerator level flag. */
  getIsUserModerator: createSelector(
    [({ server }: State) => server.user],
    (user): boolean => {
      if (!user) {
        return false;
      }
      const mask = Data.ServerInfo_User_UserLevelFlag.IsModerator;
      return (user.userLevel & mask) === mask;
    }
  ),
  getLogs: ({ server }: State) => server.logs,
  getBackendDecks: ({ server }: State) => server.backendDecks,
  getDownloadedDeck: ({ server }: State) => server.downloadedDeck,
  getDownloadedReplay: ({ server }: State) => server.downloadedReplay,
  getRegistrationError: ({ server }: State) => server.registrationError,
  getSortUsersBy: ({ server }: State) => server.sortUsersBy,

  /** Raw keyed maps — use the sorted-view selectors below for display. */
  getUsers: ({ server }: State) => server.users,
  getBuddyList: ({ server }: State) => server.buddyList,
  getIgnoreList: ({ server }: State) => server.ignoreList,
  getReplays: ({ server }: State) => server.replays,

  /**
   * Sorted array views of the keyed maps. Memoized via `createSelector` so
   * the array reference is stable until the underlying map or sort config
   * actually changes — consumers using these in `useAppSelector` won't
   * re-render unnecessarily.
   */
  getSortedUsers: createSelector(
    [(state: State) => state.server.users, (state: State) => state.server.sortUsersBy],
    (users, sortBy): Data.ServerInfo_User[] => {
      if (!users || Object.keys(users).length === 0) {
        return EMPTY_USERS;
      }
      return SortUtil.sortedUsersByField(Object.values(users), sortBy);
    }
  ),

  getSortedBuddyList: createSelector(
    [(state: State) => state.server.buddyList, (state: State) => state.server.sortUsersBy],
    (buddyList, sortBy): Data.ServerInfo_User[] => {
      if (!buddyList || Object.keys(buddyList).length === 0) {
        return EMPTY_USERS;
      }
      return SortUtil.sortedUsersByField(Object.values(buddyList), sortBy);
    }
  ),

  getSortedIgnoreList: createSelector(
    [(state: State) => state.server.ignoreList, (state: State) => state.server.sortUsersBy],
    (ignoreList, sortBy): Data.ServerInfo_User[] => {
      if (!ignoreList || Object.keys(ignoreList).length === 0) {
        return EMPTY_USERS;
      }
      return SortUtil.sortedUsersByField(Object.values(ignoreList), sortBy);
    }
  ),

  /** Replay list as an array, ordered by gameId ascending for stable display. */
  getReplaysList: createSelector(
    [(state: State) => state.server.replays],
    (replays): Data.ServerInfo_ReplayMatch[] => {
      if (!replays || Object.keys(replays).length === 0) {
        return EMPTY_REPLAYS;
      }
      return Object.values(replays).sort((a, b) => a.gameId - b.gameId);
    }
  ),
}
