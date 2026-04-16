import { App, Data } from '@app/types';
import { create } from '@bufbuild/protobuf';
import { serverReducer } from './server.reducer';
import { Actions } from './server.actions';
import {
  makeBanHistoryItem,
  makePendingActivationContext,
  makeDeckList,
  makeDeckTreeItem,
  makeGame,
  makeLogItem,
  makeReplayMatch,
  makeServerState,
  makeUser,
  makeWarnHistoryItem,
  makeWarnListItem,
} from './__mocks__/server-fixtures';

const UserLevelFlag = Data.ServerInfo_User_UserLevelFlag;

// ── Initialisation ───────────────────────────────────────────────────────────

describe('Initialisation', () => {
  it('returns initialState when called with undefined state', () => {
    const result = serverReducer(undefined, { type: '@@INIT' });
    expect(result.initialized).toBe(false);
    expect(result.buddyList).toEqual({});
    expect(result.status.state).toBe(App.StatusEnum.DISCONNECTED);
  });

  it('INITIALIZED → resets to initialState with initialized: true', () => {
    const state = makeServerState({ banUser: 'someone', initialized: false });
    const result = serverReducer(state, Actions.initialized());
    expect(result.initialized).toBe(true);
    expect(result.banUser).toBe('');
    expect(result.buddyList).toEqual({});
  });

  it('CLEAR_STORE → resets to initialState but preserves status', () => {
    const status = { state: App.StatusEnum.LOGGED_IN, description: 'logged in', connectionAttemptMade: true };
    const state = makeServerState({ status, banUser: 'someone' });
    const result = serverReducer(state, Actions.clearStore());
    expect(result.banUser).toBe('');
    expect(result.status).toEqual(status);
    expect(result.initialized).toBe(false);
  });

  it('default → returns state unchanged for unknown action', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: '@@UNKNOWN' });
    expect(result).toEqual(state);
  });
});

// ── Account & Connection ─────────────────────────────────────────────────────

describe('Account & Connection', () => {
  it('CONNECTION_ATTEMPTED → sets connectionAttemptMade to true', () => {
    const state = makeServerState({ status: { connectionAttemptMade: false, state: App.StatusEnum.DISCONNECTED, description: null } });
    const result = serverReducer(state, Actions.connectionAttempted());
    expect(result.status.connectionAttemptMade).toBe(true);
  });

  it('ACCOUNT_AWAITING_ACTIVATION → returns state unchanged', () => {
    const options = makePendingActivationContext();
    const state = makeServerState();
    const result = serverReducer(state, Actions.accountAwaitingActivation({ options }));
    expect(result).toEqual(state);
  });

  it('ACCOUNT_ACTIVATION_SUCCESS → returns state unchanged', () => {
    const state = makeServerState();
    const result = serverReducer(state, Actions.accountActivationSuccess());
    expect(result).toEqual(state);
  });

  it('ACCOUNT_ACTIVATION_FAILED → returns state unchanged', () => {
    const state = makeServerState();
    const result = serverReducer(state, Actions.accountActivationFailed());
    expect(result).toEqual(state);
  });
});

// ── Registration ──────────────────────────────────────────────────────────────

describe('Registration', () => {
  it('REGISTRATION_FAILED → stores normalized error (plain reason)', () => {
    const state = makeServerState({ registrationError: null });
    const result = serverReducer(state, Actions.registrationFailed({ reason: 'Server is disabled', endTime: undefined }));
    expect(result.registrationError).toBe('Server is disabled');
  });

  it('REGISTRATION_FAILED → normalizes banned error when endTime is given', () => {
    const state = makeServerState({ registrationError: null });
    const result = serverReducer(state, Actions.registrationFailed({ reason: 'bad actor', endTime: Date.now() + 100_000 }));
    expect(result.registrationError).toContain('banned');
    expect(result.registrationError).toContain('bad actor');
  });

  it('CLEAR_REGISTRATION_ERRORS → sets registrationError to null', () => {
    const state = makeServerState({ registrationError: 'some error' });
    const result = serverReducer(state, Actions.clearRegistrationErrors());
    expect(result.registrationError).toBeNull();
  });

  it('CLEAR_STORE → resets registrationError to null', () => {
    const state = makeServerState({ registrationError: 'stale error' });
    const result = serverReducer(state, Actions.clearStore());
    expect(result.registrationError).toBeNull();
  });
});

// ── Server Info & Status ──────────────────────────────────────────────────────

describe('Server Info & Status', () => {
  it('SERVER_MESSAGE → merges message into state.info', () => {
    const state = makeServerState({ info: { message: null, name: 'Old', version: '1.0' } });
    const result = serverReducer(state, Actions.serverMessage({ message: 'Welcome!' }));
    expect(result.info.message).toBe('Welcome!');
    expect(result.info.name).toBe('Old');
    expect(result.info.version).toBe('1.0');
  });

  it('UPDATE_INFO → merges name and version into state.info (not message)', () => {
    const state = makeServerState({ info: { message: 'hi', name: null, version: null } });
    const result = serverReducer(state, Actions.updateInfo({ info: { name: 'Servatrice', version: '2.9.0' } }));
    expect(result.info.name).toBe('Servatrice');
    expect(result.info.version).toBe('2.9.0');
    expect(result.info.message).toBe('hi');
  });

  it('UPDATE_STATUS → merges state and description into status', () => {
    const state = makeServerState();
    const update = { state: App.StatusEnum.LOGGED_IN, description: 'ok' };
    const result = serverReducer(state, Actions.updateStatus({ status: update }));
    expect(result.status.state).toBe(App.StatusEnum.LOGGED_IN);
    expect(result.status.description).toBe('ok');
    expect(result.status.connectionAttemptMade).toBe(false);
  });
});

// ── User ──────────────────────────────────────────────────────────────────────

describe('User', () => {
  it('UPDATE_USER → merges action.payload.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice', userLevel: 1 }) });
    const result = serverReducer(state, Actions.updateUser({ user: { userLevel: 8 } as any }));
    expect(result.user.name).toBe('Alice');
    expect(result.user.userLevel).toBe(8);
  });

  it('ACCOUNT_EDIT_CHANGED → merges action.payload.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }) });
    const result = serverReducer(state, Actions.accountEditChanged({ user: { realName: 'Alice Smith' } }));
    expect(result.user.realName).toBe('Alice Smith');
    expect(result.user.name).toBe('Alice');
  });

  it('ACCOUNT_IMAGE_CHANGED → merges action.payload.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }) });
    const result = serverReducer(state, Actions.accountImageChanged({ user: { country: 'US' } }));
    expect(result.user.country).toBe('US');
  });
});

// ── Users List ────────────────────────────────────────────────────────────────

describe('Users List', () => {
  it('UPDATE_USERS → replaces users map keyed by name', () => {
    const state = makeServerState();
    const users = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, Actions.updateUsers({ users }));
    expect(result.users['Alice']).toBeDefined();
    expect(result.users['Zane']).toBeDefined();
    expect(Object.keys(result.users)).toHaveLength(2);
  });

  it('USER_JOINED → inserts user into map', () => {
    const state = makeServerState({ users: { Zane: makeUser({ name: 'Zane' }) } });
    const result = serverReducer(state, Actions.userJoined({ user: makeUser({ name: 'Alice' }) }));
    expect(result.users['Alice']).toBeDefined();
    expect(result.users['Zane']).toBeDefined();
  });

  it('USER_LEFT → removes user by name from map', () => {
    const state = makeServerState({
      users: { Alice: makeUser({ name: 'Alice' }), Bob: makeUser({ name: 'Bob' }) },
    });
    const result = serverReducer(state, Actions.userLeft({ name: 'Alice' }));
    expect(result.users['Alice']).toBeUndefined();
    expect(result.users['Bob']).toBeDefined();
  });
});

// ── Buddy & Ignore Lists ──────────────────────────────────────────────────────

describe('Buddy List', () => {
  it('UPDATE_BUDDY_LIST → replaces map keyed by name', () => {
    const state = makeServerState();
    const buddyList = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, Actions.updateBuddyList({ buddyList }));
    expect(result.buddyList['Alice']).toBeDefined();
    expect(result.buddyList['Zane']).toBeDefined();
  });

  it('ADD_TO_BUDDY_LIST → inserts user into map', () => {
    const state = makeServerState({ buddyList: { Zane: makeUser({ name: 'Zane' }) } });
    const result = serverReducer(state, Actions.addToBuddyList({ user: makeUser({ name: 'Alice' }) }));
    expect(result.buddyList['Alice']).toBeDefined();
    expect(Object.keys(result.buddyList)).toHaveLength(2);
  });

  it('REMOVE_FROM_BUDDY_LIST → removes user by name from map', () => {
    const state = makeServerState({
      buddyList: { Alice: makeUser({ name: 'Alice' }), Bob: makeUser({ name: 'Bob' }) },
    });
    const result = serverReducer(state, Actions.removeFromBuddyList({ userName: 'Alice' }));
    expect(result.buddyList['Alice']).toBeUndefined();
    expect(result.buddyList['Bob']).toBeDefined();
  });
});

describe('Ignore List', () => {
  it('UPDATE_IGNORE_LIST → replaces map keyed by name', () => {
    const state = makeServerState();
    const ignoreList = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, Actions.updateIgnoreList({ ignoreList }));
    expect(result.ignoreList['Alice']).toBeDefined();
    expect(result.ignoreList['Zane']).toBeDefined();
  });

  it('ADD_TO_IGNORE_LIST → inserts user into map', () => {
    const state = makeServerState({ ignoreList: { Zane: makeUser({ name: 'Zane' }) } });
    const result = serverReducer(state, Actions.addToIgnoreList({ user: makeUser({ name: 'Alice' }) }));
    expect(result.ignoreList['Alice']).toBeDefined();
    expect(Object.keys(result.ignoreList)).toHaveLength(2);
  });

  it('REMOVE_FROM_IGNORE_LIST → removes user by name from map', () => {
    const state = makeServerState({
      ignoreList: { Alice: makeUser({ name: 'Alice' }), Bob: makeUser({ name: 'Bob' }) },
    });
    const result = serverReducer(state, Actions.removeFromIgnoreList({ userName: 'Alice' }));
    expect(result.ignoreList['Alice']).toBeUndefined();
    expect(result.ignoreList['Bob']).toBeDefined();
  });
});

// ── Logs ─────────────────────────────────────────────────────────────────────

describe('Logs', () => {
  it('VIEW_LOGS → groups LogItem[] into room/game/chat buckets', () => {
    const log = makeLogItem({ targetType: 'room' });
    const state = makeServerState();
    const result = serverReducer(state, Actions.viewLogs({ logs: [log] }));
    expect(result.logs.room).toEqual([log]);
  });

  it('CLEAR_LOGS → resets logs to empty arrays', () => {
    const state = makeServerState({ logs: { room: [makeLogItem()], game: [], chat: [] } });
    const result = serverReducer(state, Actions.clearLogs());
    expect(result.logs.room).toEqual([]);
    expect(result.logs.game).toEqual([]);
    expect(result.logs.chat).toEqual([]);
  });
});

// ── Messaging ─────────────────────────────────────────────────────────────────

describe('Messaging', () => {
  it('USER_MESSAGE → uses receiverName as key when current user is sender', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }), messages: {} });
    const messageData = { senderName: 'Alice', receiverName: 'Bob', message: 'hi' } as Data.Event_UserMessage;
    const result = serverReducer(state, Actions.userMessage({ messageData }));
    expect(result.messages['Bob']).toHaveLength(1);
    expect(result.messages['Bob'][0]).toEqual(messageData);
  });

  it('USER_MESSAGE → uses senderName as key when current user is receiver', () => {
    const state = makeServerState({ user: makeUser({ name: 'Bob' }), messages: {} });
    const messageData = { senderName: 'Alice', receiverName: 'Bob', message: 'yo' } as Data.Event_UserMessage;
    const result = serverReducer(state, Actions.userMessage({ messageData }));
    expect(result.messages['Alice']).toHaveLength(1);
    expect(result.messages['Alice'][0]).toEqual(messageData);
  });

  it('USER_MESSAGE → appends to existing messages for that user', () => {
    const existingMsg = create(Data.Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'first' });
    const state = makeServerState({
      user: makeUser({ name: 'Bob' }),
      messages: { Alice: [existingMsg] },
    });
    const newMsg = create(Data.Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'second' });
    const result = serverReducer(state, Actions.userMessage({ messageData: newMsg }));
    expect(result.messages['Alice']).toHaveLength(2);
  });
});

// ── User Info & Notifications ─────────────────────────────────────────────────

describe('User Info & Notifications', () => {
  it('GET_USER_INFO → adds userInfo keyed by name', () => {
    const userInfo = makeUser({ name: 'Eve' });
    const state = makeServerState();
    const result = serverReducer(state, Actions.getUserInfo({ userInfo }));
    expect(result.userInfo['Eve']).toEqual(userInfo);
  });

  it('NOTIFY_USER → appends notification to list', () => {
    const state = makeServerState({ notifications: [] });
    const notification = { type: 1, warningReason: '', customTitle: '', customContent: '' } as unknown as Data.Event_NotifyUser;
    const result = serverReducer(state, Actions.notifyUser({ notification }));
    expect(result.notifications).toHaveLength(1);
    expect(result.notifications[0]).toEqual(notification);
  });

  it('SERVER_SHUTDOWN → sets serverShutdown to action.payload.data', () => {
    const data = { reason: 'maintenance', minutes: 10 } as unknown as Data.Event_ServerShutdown;
    const state = makeServerState();
    const result = serverReducer(state, Actions.serverShutdown({ data }));
    expect(result.serverShutdown).toEqual(data);
  });
});

// ── Moderation ────────────────────────────────────────────────────────────────

describe('Moderation', () => {
  it('BAN_FROM_SERVER → sets banUser', () => {
    const state = makeServerState();
    const result = serverReducer(state, Actions.banFromServer({ userName: 'Frank' }));
    expect(result.banUser).toBe('Frank');
  });

  it('BAN_HISTORY → adds banHistory keyed by userName', () => {
    const history = [makeBanHistoryItem()];
    const state = makeServerState();
    const result = serverReducer(state, Actions.banHistory({ userName: 'Frank', banHistory: history }));
    expect(result.banHistory['Frank']).toEqual(history);
  });

  it('WARN_HISTORY → adds warnHistory keyed by userName', () => {
    const history = [makeWarnHistoryItem()];
    const state = makeServerState();
    const result = serverReducer(state, Actions.warnHistory({ userName: 'Grace', warnHistory: history }));
    expect(result.warnHistory['Grace']).toEqual(history);
  });

  it('WARN_LIST_OPTIONS → replaces warnListOptions', () => {
    const list = [makeWarnListItem()];
    const state = makeServerState();
    const result = serverReducer(state, Actions.warnListOptions({ warnList: list }));
    expect(result.warnListOptions).toEqual(list);
  });

  it('WARN_USER → sets warnUser', () => {
    const state = makeServerState();
    const result = serverReducer(state, Actions.warnUser({ userName: 'Hank' }));
    expect(result.warnUser).toBe('Hank');
  });

  it('GET_ADMIN_NOTES → adds adminNotes keyed by userName', () => {
    const state = makeServerState();
    const result = serverReducer(state, Actions.getAdminNotes({ userName: 'Ira', notes: 'note1' }));
    expect(result.adminNotes['Ira']).toBe('note1');
  });

  it('UPDATE_ADMIN_NOTES → updates adminNotes keyed by userName', () => {
    const state = makeServerState({ adminNotes: { Ira: 'old' } });
    const result = serverReducer(state, Actions.updateAdminNotes({ userName: 'Ira', notes: 'new' }));
    expect(result.adminNotes['Ira']).toBe('new');
  });
});

// ── ADJUST_MOD ────────────────────────────────────────────────────────────────

describe('ADJUST_MOD', () => {
  const baseUserLevel = UserLevelFlag.IsUser | UserLevelFlag.IsRegistered | UserLevelFlag.IsModerator | UserLevelFlag.IsJudge;

  it('shouldBeMod=true, shouldBeJudge=true → sets both bits, preserves IsUser|IsRegistered', () => {
    const state = makeServerState({ users: { Dan: makeUser({ name: 'Dan', userLevel: baseUserLevel }) } });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: true, shouldBeJudge: true }));
    // IsUser(1) | IsRegistered(2) | IsModerator(4) | IsJudge(16) = 23
    expect(result.users['Dan'].userLevel).toBe(23);
  });

  it('shouldBeMod=true, shouldBeJudge=false → sets IsModerator, clears IsJudge, preserves others', () => {
    const state = makeServerState({ users: { Dan: makeUser({ name: 'Dan', userLevel: baseUserLevel }) } });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: true, shouldBeJudge: false }));
    // IsUser(1) | IsRegistered(2) | IsModerator(4) = 7
    expect(result.users['Dan'].userLevel).toBe(7);
  });

  it('shouldBeMod=false, shouldBeJudge=true → clears IsModerator, sets IsJudge, preserves others', () => {
    const state = makeServerState({ users: { Dan: makeUser({ name: 'Dan', userLevel: baseUserLevel }) } });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: false, shouldBeJudge: true }));
    // IsUser(1) | IsRegistered(2) | IsJudge(16) = 19
    expect(result.users['Dan'].userLevel).toBe(19);
  });

  it('shouldBeMod=false, shouldBeJudge=false → clears both bits, preserves IsUser|IsRegistered', () => {
    const state = makeServerState({ users: { Dan: makeUser({ name: 'Dan', userLevel: baseUserLevel }) } });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: false, shouldBeJudge: false }));
    // IsUser(1) | IsRegistered(2) = 3
    expect(result.users['Dan'].userLevel).toBe(3);
  });

  it('shouldBeMod=true on IsUser|IsRegistered only → produces 7, not 4', () => {
    const state = makeServerState({
      users: { Dan: makeUser({ name: 'Dan', userLevel: UserLevelFlag.IsUser | UserLevelFlag.IsRegistered }) },
    });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: true, shouldBeJudge: false }));
    // IsUser(1) | IsRegistered(2) | IsModerator(4) = 7
    expect(result.users['Dan'].userLevel).toBe(7);
  });

  it('non-matching users are left unchanged', () => {
    const alice = makeUser({ name: 'Alice', userLevel: 7 });
    const state = makeServerState({
      users: { Alice: alice, Dan: makeUser({ name: 'Dan', userLevel: baseUserLevel }) },
    });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Dan', shouldBeMod: false, shouldBeJudge: false }));
    expect(result.users['Alice']).toEqual(alice);
  });

  it('unknown userName → state unchanged', () => {
    const state = makeServerState({ users: { Dan: makeUser({ name: 'Dan' }) } });
    const result = serverReducer(state, Actions.adjustMod({ userName: 'Ghost', shouldBeMod: true, shouldBeJudge: false }));
    expect(result).toEqual(state);
  });
});

// ── Replays ───────────────────────────────────────────────────────────────────

describe('Replays', () => {
  it('REPLAY_LIST → replaces replays map keyed by gameId', () => {
    const matchList = [makeReplayMatch({ gameId: 10 })];
    const state = makeServerState({ replays: { 99: makeReplayMatch({ gameId: 99 }) } });
    const result = serverReducer(state, Actions.replayList({ matchList }));
    expect(Object.keys(result.replays)).toHaveLength(1);
    expect(result.replays[10]).toBeDefined();
    expect(result.replays[99]).toBeUndefined();
  });

  it('REPLAY_ADDED → inserts matchInfo into replays map', () => {
    const existing = makeReplayMatch({ gameId: 1 });
    const added = makeReplayMatch({ gameId: 2 });
    const state = makeServerState({ replays: { 1: existing } });
    const result = serverReducer(state, Actions.replayAdded({ matchInfo: added }));
    expect(Object.keys(result.replays)).toHaveLength(2);
    expect(result.replays[2]).toEqual(added);
  });

  it('REPLAY_MODIFY_MATCH → updates doNotHide for matching gameId', () => {
    const state = makeServerState({ replays: { 5: makeReplayMatch({ gameId: 5, doNotHide: false }) } });
    const result = serverReducer(state, Actions.replayModifyMatch({ gameId: 5, doNotHide: true }));
    expect(result.replays[5].doNotHide).toBe(true);
  });

  it('REPLAY_MODIFY_MATCH → leaves non-matching replays unchanged', () => {
    const r1 = makeReplayMatch({ gameId: 1, doNotHide: false });
    const r2 = makeReplayMatch({ gameId: 2, doNotHide: false });
    const state = makeServerState({ replays: { 1: r1, 2: r2 } });
    const result = serverReducer(state, Actions.replayModifyMatch({ gameId: 1, doNotHide: true }));
    expect(result.replays[2]).toEqual(r2);
    expect(result.replays[2].doNotHide).toBe(false);
  });

  it('REPLAY_MODIFY_MATCH → unknown gameId → state unchanged', () => {
    const state = makeServerState({ replays: { 5: makeReplayMatch({ gameId: 5 }) } });
    const result = serverReducer(state, Actions.replayModifyMatch({ gameId: 999, doNotHide: true }));
    expect(result).toEqual(state);
  });

  it('REPLAY_DELETE_MATCH → removes replay by gameId', () => {
    const state = makeServerState({
      replays: { 5: makeReplayMatch({ gameId: 5 }), 6: makeReplayMatch({ gameId: 6 }) },
    });
    const result = serverReducer(state, Actions.replayDeleteMatch({ gameId: 5 }));
    expect(Object.keys(result.replays)).toHaveLength(1);
    expect(result.replays[5]).toBeUndefined();
    expect(result.replays[6]).toBeDefined();
  });
});

// ── Deck Storage ──────────────────────────────────────────────────────────────

describe('Deck Storage', () => {
  it('BACKEND_DECKS → sets backendDecks', () => {
    const deckList = makeDeckList();
    const state = makeServerState();
    const result = serverReducer(state, Actions.backendDecks({ deckList }));
    expect(result.backendDecks).toEqual(deckList);
  });

  it('DECK_UPLOAD with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, Actions.deckUpload({ path: '', treeItem: makeDeckTreeItem() }));
    expect(result).toEqual(state);
  });

  it('DECK_UPLOAD with flat path → appends item to root', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const item = makeDeckTreeItem({ name: 'deck.cod' });
    const result = serverReducer(state, Actions.deckUpload({ path: '', treeItem: item }));
    expect(result.backendDecks!.root!.items).toHaveLength(1);
    expect(result.backendDecks!.root!.items[0]).toEqual(item);
  });

  it('DECK_UPLOAD with nested path → inserts into matching subfolder', () => {
    const subfolder = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'myDecks', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const item = makeDeckTreeItem({ name: 'new.cod' });
    const result = serverReducer(state, Actions.deckUpload({ path: 'myDecks', treeItem: item }));
    const folder = result.backendDecks!.root!.items.find(i => i.name === 'myDecks');
    expect(folder!.folder!.items).toHaveLength(1);
    expect(folder!.folder!.items[0]).toEqual(item);
  });

  it('DECK_UPLOAD with non-existent intermediate folder → creates folder and inserts', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const item = makeDeckTreeItem({ name: 'deck.cod' });
    const result = serverReducer(state, Actions.deckUpload({ path: 'newFolder', treeItem: item }));
    expect(result.backendDecks!.root!.items).toHaveLength(1);
    expect(result.backendDecks!.root!.items[0].name).toBe('newFolder');
    expect(result.backendDecks!.root!.items[0].folder!.items[0]).toEqual(item);
  });

  it('DECK_DELETE with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, Actions.deckDelete({ deckId: 1 }));
    expect(result).toEqual(state);
  });

  it('DECK_DELETE → removes item by id from tree', () => {
    const item = makeDeckTreeItem({ id: 7 });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [item] }) }),
    });
    const result = serverReducer(state, Actions.deckDelete({ deckId: 7 }));
    expect(result.backendDecks!.root!.items).toHaveLength(0);
  });

  it('DECK_DELETE → recursively removes item nested inside a subfolder', () => {
    const nested = makeDeckTreeItem({ id: 9, name: 'nested.cod' });
    const subfolder = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'sub', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [nested] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, Actions.deckDelete({ deckId: 9 }));
    expect(result.backendDecks!.root!.items[0].folder!.items).toHaveLength(0);
  });

  it('DECK_NEW_DIR with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, Actions.deckNewDir({ path: '', dirName: 'newDir' }));
    expect(result).toEqual(state);
  });

  it('DECK_NEW_DIR at root → appends folder to root items', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const result = serverReducer(state, Actions.deckNewDir({ path: '', dirName: 'myDir' }));
    expect(result.backendDecks!.root!.items).toHaveLength(1);
    expect(result.backendDecks!.root!.items[0].name).toBe('myDir');
    expect(result.backendDecks!.root!.items[0].folder!.items).toEqual([]);
  });

  it('DECK_NEW_DIR nested → inserts folder inside matching subfolder', () => {
    const subfolder = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'parent', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, Actions.deckNewDir({ path: 'parent', dirName: 'child' }));
    const parent = result.backendDecks!.root!.items.find(i => i.name === 'parent');
    expect(parent!.folder!.items).toHaveLength(1);
    expect(parent!.folder!.items[0].name).toBe('child');
  });

  it('DECK_DEL_DIR with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, Actions.deckDelDir({ path: 'myDir' }));
    expect(result).toEqual(state);
  });

  it('DECK_DEL_DIR → removes folder from root by name', () => {
    const subfolder = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'myDir', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, Actions.deckDelDir({ path: 'myDir' }));
    expect(result.backendDecks!.root!.items).toHaveLength(0);
  });

  it('DECK_DEL_DIR → returns deck tree unchanged when path is empty', () => {
    const subfolder = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'keep', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, Actions.deckDelDir({ path: '' }));
    expect(result.backendDecks!.root!.items).toHaveLength(1);
  });

  it('DECK_DEL_DIR → recursively removes nested subfolder via multi-segment path', () => {
    const child = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'child', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const parent = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'parent', folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [child] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [parent] }) })
    });
    const result = serverReducer(state, Actions.deckDelDir({ path: 'parent/child' }));
    expect(result.backendDecks!.root!.items[0].folder!.items).toHaveLength(0);
  });
});

// ── GAMES_OF_USER ─────────────────────────────────────────────────────────────

describe('GAMES_OF_USER', () => {
  it('stores normalized games keyed by userName', () => {
    const response = create(Data.Response_GetGamesOfUserSchema, {
      gameList: [create(Data.ServerInfo_GameSchema, { gameId: 5, description: '' })],
      roomList: [],
    });
    const state = makeServerState();
    const result = serverReducer(state, Actions.gamesOfUser({ userName: 'alice', response }));
    expect(result.gamesOfUser['alice']).toEqual([makeGame({ gameId: 5 })]);
  });

  it('overwrites previous games for same user', () => {
    const old = [makeGame({ gameId: 1 })];
    const response = create(Data.Response_GetGamesOfUserSchema, {
      gameList: [create(Data.ServerInfo_GameSchema, { gameId: 2, description: '' })],
      roomList: [],
    });
    const state = makeServerState({ gamesOfUser: { alice: old } });
    const result = serverReducer(state, Actions.gamesOfUser({ userName: 'alice', response }));
    expect(result.gamesOfUser['alice']).toEqual([makeGame({ gameId: 2 })]);
  });

  it('does not affect other users\' entries', () => {
    const bobGames = [makeGame({ gameId: 3 })];
    const response = create(Data.Response_GetGamesOfUserSchema, { gameList: [], roomList: [] });
    const state = makeServerState({ gamesOfUser: { bob: bobGames } });
    const result = serverReducer(state, Actions.gamesOfUser({ userName: 'alice', response }));
    expect(result.gamesOfUser['bob']).toEqual(bobGames);
  });
});
