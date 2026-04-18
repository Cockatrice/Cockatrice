import { Selectors } from './server.selectors';
import { ServerState } from './server.interfaces';
import {
  makeDeckList,
  makeReplayMatch,
  makeServerState,
  makeUser,
} from './__mocks__/server-fixtures';
import { Data, Enriched } from '@app/types';

function rootState(server: ServerState) {
  return { server };
}

describe('Selectors', () => {
  it('getInitialized → returns initialized flag', () => {
    const state = makeServerState({ initialized: true });
    expect(Selectors.getInitialized(rootState(state))).toBe(true);
  });

  it('getMessage → returns info.message', () => {
    const state = makeServerState({ info: { message: 'Welcome!', name: null, version: null } });
    expect(Selectors.getMessage(rootState(state))).toBe('Welcome!');
  });

  it('getName → returns info.name', () => {
    const state = makeServerState({ info: { message: null, name: 'Servatrice', version: null } });
    expect(Selectors.getName(rootState(state))).toBe('Servatrice');
  });

  it('getVersion → returns info.version', () => {
    const state = makeServerState({ info: { message: null, name: null, version: '2.9.0' } });
    expect(Selectors.getVersion(rootState(state))).toBe('2.9.0');
  });

  it('getDescription → returns status.description', () => {
    const state = makeServerState({ status: { connectionAttemptMade: false, state: Enriched.StatusEnum.CONNECTED, description: 'ok' } });
    expect(Selectors.getDescription(rootState(state))).toBe('ok');
  });

  it('getState → returns status.state', () => {
    const state = makeServerState({ status: { connectionAttemptMade: false, state: Enriched.StatusEnum.LOGGED_IN, description: null } });
    expect(Selectors.getState(rootState(state))).toBe(Enriched.StatusEnum.LOGGED_IN);
  });

  it('getConnectionAttemptMade → returns status.connectionAttemptMade', () => {
    const state = makeServerState({ status: { connectionAttemptMade: true, state: Enriched.StatusEnum.DISCONNECTED, description: null } });
    expect(Selectors.getConnectionAttemptMade(rootState(state))).toBe(true);
  });

  it('getUser → returns user', () => {
    const user = makeUser({ name: 'Alice' });
    const state = makeServerState({ user });
    expect(Selectors.getUser(rootState(state))).toBe(user);
  });

  it('getUsers → returns users keyed map', () => {
    const users = { TestUser: makeUser(), Bob: makeUser({ name: 'Bob' }) };
    const state = makeServerState({ users });
    expect(Selectors.getUsers(rootState(state))).toBe(users);
  });

  it('getLogs → returns logs object', () => {
    const logs = { room: [], game: [], chat: [] };
    const state = makeServerState({ logs });
    expect(Selectors.getLogs(rootState(state))).toBe(logs);
  });

  it('getBuddyList → returns buddyList keyed map', () => {
    const buddyList = { Carol: makeUser({ name: 'Carol' }) };
    const state = makeServerState({ buddyList });
    expect(Selectors.getBuddyList(rootState(state))).toBe(buddyList);
  });

  it('getIgnoreList → returns ignoreList keyed map', () => {
    const ignoreList = { Dave: makeUser({ name: 'Dave' }) };
    const state = makeServerState({ ignoreList });
    expect(Selectors.getIgnoreList(rootState(state))).toBe(ignoreList);
  });

  it('getReplays → returns replays keyed map', () => {
    const replays = { 1: makeReplayMatch() };
    const state = makeServerState({ replays });
    expect(Selectors.getReplays(rootState(state))).toBe(replays);
  });

  it('getSortedUsers → returns user array sorted by name ASC', () => {
    const users = { Zane: makeUser({ name: 'Zane' }), Alice: makeUser({ name: 'Alice' }) };
    const state = makeServerState({ users });
    const sorted = Selectors.getSortedUsers(rootState(state));
    expect(sorted[0].name).toBe('Alice');
    expect(sorted[1].name).toBe('Zane');
  });

  it('getSortedUsers → returns EMPTY_USERS for empty map', () => {
    const state = makeServerState({ users: {} });
    const sorted = Selectors.getSortedUsers(rootState(state));
    expect(sorted).toHaveLength(0);
  });

  it('getSortedBuddyList → returns buddy array sorted by name ASC', () => {
    const buddyList = { Zane: makeUser({ name: 'Zane' }), Alice: makeUser({ name: 'Alice' }) };
    const state = makeServerState({ buddyList });
    const sorted = Selectors.getSortedBuddyList(rootState(state));
    expect(sorted[0].name).toBe('Alice');
    expect(sorted[1].name).toBe('Zane');
  });

  it('getSortedIgnoreList → returns ignore array sorted by name ASC', () => {
    const ignoreList = { Zane: makeUser({ name: 'Zane' }), Alice: makeUser({ name: 'Alice' }) };
    const state = makeServerState({ ignoreList });
    const sorted = Selectors.getSortedIgnoreList(rootState(state));
    expect(sorted[0].name).toBe('Alice');
    expect(sorted[1].name).toBe('Zane');
  });

  it('getReplaysList → returns replay array sorted by gameId ASC', () => {
    const replays = { 10: makeReplayMatch({ gameId: 10 }), 3: makeReplayMatch({ gameId: 3 }) };
    const state = makeServerState({ replays });
    const sorted = Selectors.getReplaysList(rootState(state));
    expect(sorted[0].gameId).toBe(3);
    expect(sorted[1].gameId).toBe(10);
  });

  it('getBackendDecks → returns backendDecks', () => {
    const backendDecks = makeDeckList();
    const state = makeServerState({ backendDecks });
    expect(Selectors.getBackendDecks(rootState(state))).toBe(backendDecks);
  });

  it('getBackendDecks → returns null when not set', () => {
    const state = makeServerState({ backendDecks: null });
    expect(Selectors.getBackendDecks(rootState(state))).toBeNull();
  });

  it('getDownloadedDeck → returns downloadedDeck', () => {
    const downloadedDeck = { deckId: 42, deck: '<xml>' };
    const state = makeServerState({ downloadedDeck });
    expect(Selectors.getDownloadedDeck(rootState(state))).toEqual(downloadedDeck);
  });

  it('getDownloadedReplay → returns downloadedReplay', () => {
    const downloadedReplay = { replayId: 99, replayData: new Uint8Array([1, 2, 3]) };
    const state = makeServerState({ downloadedReplay });
    expect(Selectors.getDownloadedReplay(rootState(state))).toEqual(downloadedReplay);
  });

  it('getRegistrationError → returns registrationError', () => {
    const state = makeServerState({ registrationError: 'bad input' });
    expect(Selectors.getRegistrationError(rootState(state))).toBe('bad input');
  });

  // ── derived selectors (createSelector) ──────────────────────────────

  it('getIsConnected → true when state is LOGGED_IN', () => {
    const state = makeServerState({ status: { connectionAttemptMade: true, state: Enriched.StatusEnum.LOGGED_IN, description: null } });
    expect(Selectors.getIsConnected(rootState(state))).toBe(true);
  });

  it('getIsConnected → false when state is CONNECTED', () => {
    const state = makeServerState({ status: { connectionAttemptMade: true, state: Enriched.StatusEnum.CONNECTED, description: null } });
    expect(Selectors.getIsConnected(rootState(state))).toBe(false);
  });

  it('getIsConnected → false when state is DISCONNECTED', () => {
    const state = makeServerState({ status: { connectionAttemptMade: false, state: Enriched.StatusEnum.DISCONNECTED, description: null } });
    expect(Selectors.getIsConnected(rootState(state))).toBe(false);
  });

  it('getIsUserModerator → true when user has IsModerator flag', () => {
    const Flag = Data.ServerInfo_User_UserLevelFlag;
    const user = makeUser({ userLevel: Flag.IsUser | Flag.IsModerator });
    const state = makeServerState({ user });
    expect(Selectors.getIsUserModerator(rootState(state))).toBe(true);
  });

  it('getIsUserModerator → false when user lacks IsModerator flag', () => {
    const Flag = Data.ServerInfo_User_UserLevelFlag;
    const user = makeUser({ userLevel: Flag.IsUser | Flag.IsRegistered });
    const state = makeServerState({ user });
    expect(Selectors.getIsUserModerator(rootState(state))).toBe(false);
  });

  it('getIsUserModerator → false when user is null', () => {
    const state = makeServerState({ user: null });
    expect(Selectors.getIsUserModerator(rootState(state))).toBe(false);
  });

  // ── createSelector reference stability ──────────────────────────────

  it('getIsConnected → returns same value reference for identical state', () => {
    const state = makeServerState({ status: { connectionAttemptMade: true, state: Enriched.StatusEnum.LOGGED_IN, description: null } });
    const root = rootState(state);
    const a = Selectors.getIsConnected(root);
    const b = Selectors.getIsConnected(root);
    expect(a).toBe(b);
  });

  it('getSortedUsers → returns same array reference for identical state', () => {
    const users = { Alice: makeUser({ name: 'Alice' }), Bob: makeUser({ name: 'Bob' }) };
    const state = makeServerState({ users });
    const root = rootState(state);
    const a = Selectors.getSortedUsers(root);
    const b = Selectors.getSortedUsers(root);
    expect(a).toBe(b);
  });

  it('getSortedBuddyList → returns same array reference for identical state', () => {
    const buddyList = { Alice: makeUser({ name: 'Alice' }) };
    const state = makeServerState({ buddyList });
    const root = rootState(state);
    const a = Selectors.getSortedBuddyList(root);
    const b = Selectors.getSortedBuddyList(root);
    expect(a).toBe(b);
  });

  it('getSortedIgnoreList → returns same array reference for identical state', () => {
    const ignoreList = { Troll: makeUser({ name: 'Troll' }) };
    const state = makeServerState({ ignoreList });
    const root = rootState(state);
    const a = Selectors.getSortedIgnoreList(root);
    const b = Selectors.getSortedIgnoreList(root);
    expect(a).toBe(b);
  });

  it('getReplaysList → returns same array reference for identical state', () => {
    const replays = { 1: makeReplayMatch({ gameId: 1 }) };
    const state = makeServerState({ replays });
    const root = rootState(state);
    const a = Selectors.getReplaysList(root);
    const b = Selectors.getReplaysList(root);
    expect(a).toBe(b);
  });
});
