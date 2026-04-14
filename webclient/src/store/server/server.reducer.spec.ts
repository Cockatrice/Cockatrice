import { StatusEnum } from 'types';
import { ServerInfo_User_UserLevelFlag as UserLevelFlag } from 'generated/proto/serverinfo_user_pb';
import { create } from '@bufbuild/protobuf';
import { Event_UserMessageSchema } from 'generated/proto/event_user_message_pb';
import { ServerInfo_DeckStorage_FolderSchema, ServerInfo_DeckStorage_TreeItemSchema } from 'generated/proto/serverinfo_deckstorage_pb';
import { serverReducer } from './server.reducer';
import { Types } from './server.types';
import {
  makeBanHistoryItem,
  makeConnectOptions,
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

// ── Initialisation ───────────────────────────────────────────────────────────

describe('Initialisation', () => {
  it('returns initialState when called with undefined state', () => {
    const result = serverReducer(undefined, { type: '@@INIT' });
    expect(result.initialized).toBe(false);
    expect(result.buddyList).toEqual([]);
    expect(result.status.state).toBe(StatusEnum.DISCONNECTED);
  });

  it('INITIALIZED → resets to initialState with initialized: true', () => {
    const state = makeServerState({ banUser: 'someone', initialized: false });
    const result = serverReducer(state, { type: Types.INITIALIZED });
    expect(result.initialized).toBe(true);
    expect(result.banUser).toBe('');
    expect(result.buddyList).toEqual([]);
  });

  it('CLEAR_STORE → resets to initialState but preserves status', () => {
    const status = { state: StatusEnum.LOGGED_IN, description: 'logged in' };
    const state = makeServerState({ status, banUser: 'someone' });
    const result = serverReducer(state, { type: Types.CLEAR_STORE });
    expect(result.banUser).toBe('');
    expect(result.status).toEqual(status);
    expect(result.initialized).toBe(false);
  });

  it('default → returns state unchanged for unknown action', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: '@@UNKNOWN' });
    expect(result).toBe(state);
  });
});

// ── Account & Connection ─────────────────────────────────────────────────────

describe('Account & Connection', () => {
  it('CONNECTION_ATTEMPTED → sets connectionAttemptMade to true', () => {
    const state = makeServerState({ status: { connectionAttemptMade: false, state: StatusEnum.DISCONNECTED, description: null } });
    const result = serverReducer(state, { type: Types.CONNECTION_ATTEMPTED });
    expect(result.status.connectionAttemptMade).toBe(true);
  });

  it('ACCOUNT_AWAITING_ACTIVATION → returns state unchanged', () => {
    const options = makeConnectOptions();
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.ACCOUNT_AWAITING_ACTIVATION, options });
    expect(result).toBe(state);
  });

  it('ACCOUNT_ACTIVATION_SUCCESS → returns state unchanged', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.ACCOUNT_ACTIVATION_SUCCESS });
    expect(result).toBe(state);
  });

  it('ACCOUNT_ACTIVATION_FAILED → returns state unchanged', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.ACCOUNT_ACTIVATION_FAILED });
    expect(result).toBe(state);
  });
});

// ── Registration ──────────────────────────────────────────────────────────────

describe('Registration', () => {
  it('REGISTRATION_FAILED → stores normalized error (plain reason)', () => {
    const state = makeServerState({ registrationError: null });
    const result = serverReducer(state, { type: Types.REGISTRATION_FAILED, reason: 'Server is disabled', endTime: undefined });
    expect(result.registrationError).toBe('Server is disabled');
  });

  it('REGISTRATION_FAILED → normalizes banned error when endTime is given', () => {
    const state = makeServerState({ registrationError: null });
    const result = serverReducer(state, { type: Types.REGISTRATION_FAILED, reason: 'bad actor', endTime: Date.now() + 100_000 });
    expect(result.registrationError).toContain('banned');
    expect(result.registrationError).toContain('bad actor');
  });

  it('CLEAR_REGISTRATION_ERRORS → sets registrationError to null', () => {
    const state = makeServerState({ registrationError: 'some error' });
    const result = serverReducer(state, { type: Types.CLEAR_REGISTRATION_ERRORS });
    expect(result.registrationError).toBeNull();
  });

  it('CLEAR_STORE → resets registrationError to null', () => {
    const state = makeServerState({ registrationError: 'stale error' });
    const result = serverReducer(state, { type: Types.CLEAR_STORE });
    expect(result.registrationError).toBeNull();
  });
});

// ── Server Info & Status ──────────────────────────────────────────────────────

describe('Server Info & Status', () => {
  it('SERVER_MESSAGE → merges message into state.info', () => {
    const state = makeServerState({ info: { message: null, name: 'Old', version: '1.0' } });
    const result = serverReducer(state, { type: Types.SERVER_MESSAGE, message: 'Welcome!' });
    expect(result.info.message).toBe('Welcome!');
    expect(result.info.name).toBe('Old');
    expect(result.info.version).toBe('1.0');
  });

  it('UPDATE_INFO → merges name and version into state.info (not message)', () => {
    const state = makeServerState({ info: { message: 'hi', name: null, version: null } });
    const result = serverReducer(state, {
      type: Types.UPDATE_INFO,
      info: { name: 'Servatrice', version: '2.9.0' },
    });
    expect(result.info.name).toBe('Servatrice');
    expect(result.info.version).toBe('2.9.0');
    expect(result.info.message).toBe('hi');
  });

  it('UPDATE_STATUS → replaces state.status entirely', () => {
    const state = makeServerState();
    const status = { state: StatusEnum.LOGGED_IN, description: 'ok' };
    const result = serverReducer(state, { type: Types.UPDATE_STATUS, status });
    expect(result.status).toEqual(status);
  });
});

// ── User ──────────────────────────────────────────────────────────────────────

describe('User', () => {
  it('UPDATE_USER → merges action.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice', userLevel: 1 }) });
    const result = serverReducer(state, {
      type: Types.UPDATE_USER,
      user: { userLevel: 8 },
    });
    expect(result.user.name).toBe('Alice');
    expect(result.user.userLevel).toBe(8);
  });

  it('ACCOUNT_EDIT_CHANGED → merges action.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }) });
    const result = serverReducer(state, { type: Types.ACCOUNT_EDIT_CHANGED, user: { realName: 'Alice Smith' } });
    expect(result.user.realName).toBe('Alice Smith');
    expect(result.user.name).toBe('Alice');
  });

  it('ACCOUNT_IMAGE_CHANGED → merges action.user into state.user', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }) });
    const result = serverReducer(state, { type: Types.ACCOUNT_IMAGE_CHANGED, user: { country: 'US' } });
    expect(result.user.country).toBe('US');
  });
});

// ── Users List ────────────────────────────────────────────────────────────────

describe('Users List', () => {
  it('UPDATE_USERS → replaces users list and sorts by name ASC', () => {
    const state = makeServerState();
    const users = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, { type: Types.UPDATE_USERS, users });
    expect(result.users[0].name).toBe('Alice');
    expect(result.users[1].name).toBe('Zane');
  });

  it('USER_JOINED → appends user and sorts', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Zane' })] });
    const result = serverReducer(state, { type: Types.USER_JOINED, user: makeUser({ name: 'Alice' }) });
    expect(result.users[0].name).toBe('Alice');
    expect(result.users[1].name).toBe('Zane');
  });

  it('USER_LEFT → removes user by name', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Alice' }), makeUser({ name: 'Bob' })] });
    const result = serverReducer(state, { type: Types.USER_LEFT, name: 'Alice' });
    expect(result.users).toHaveLength(1);
    expect(result.users[0].name).toBe('Bob');
  });
});

// ── Buddy & Ignore Lists ──────────────────────────────────────────────────────

describe('Buddy List', () => {
  it('UPDATE_BUDDY_LIST → replaces and sorts buddy list', () => {
    const state = makeServerState();
    const buddyList = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, { type: Types.UPDATE_BUDDY_LIST, buddyList });
    expect(result.buddyList[0].name).toBe('Alice');
  });

  it('ADD_TO_BUDDY_LIST → appends user and sorts', () => {
    const state = makeServerState({ buddyList: [makeUser({ name: 'Zane' })] });
    const result = serverReducer(state, { type: Types.ADD_TO_BUDDY_LIST, user: makeUser({ name: 'Alice' }) });
    expect(result.buddyList[0].name).toBe('Alice');
    expect(result.buddyList).toHaveLength(2);
  });

  it('REMOVE_FROM_BUDDY_LIST → removes user by name', () => {
    const state = makeServerState({ buddyList: [makeUser({ name: 'Alice' }), makeUser({ name: 'Bob' })] });
    const result = serverReducer(state, { type: Types.REMOVE_FROM_BUDDY_LIST, userName: 'Alice' });
    expect(result.buddyList).toHaveLength(1);
    expect(result.buddyList[0].name).toBe('Bob');
  });
});

describe('Ignore List', () => {
  it('UPDATE_IGNORE_LIST → replaces and sorts ignore list', () => {
    const state = makeServerState();
    const ignoreList = [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })];
    const result = serverReducer(state, { type: Types.UPDATE_IGNORE_LIST, ignoreList });
    expect(result.ignoreList[0].name).toBe('Alice');
  });

  it('ADD_TO_IGNORE_LIST → appends user and sorts', () => {
    const state = makeServerState({ ignoreList: [makeUser({ name: 'Zane' })] });
    const result = serverReducer(state, { type: Types.ADD_TO_IGNORE_LIST, user: makeUser({ name: 'Alice' }) });
    expect(result.ignoreList[0].name).toBe('Alice');
    expect(result.ignoreList).toHaveLength(2);
  });

  it('REMOVE_FROM_IGNORE_LIST → removes user by name', () => {
    const state = makeServerState({ ignoreList: [makeUser({ name: 'Alice' }), makeUser({ name: 'Bob' })] });
    const result = serverReducer(state, { type: Types.REMOVE_FROM_IGNORE_LIST, userName: 'Alice' });
    expect(result.ignoreList).toHaveLength(1);
    expect(result.ignoreList[0].name).toBe('Bob');
  });
});

// ── Logs ─────────────────────────────────────────────────────────────────────

describe('Logs', () => {
  it('VIEW_LOGS → groups LogItem[] into room/game/chat buckets', () => {
    const log = makeLogItem({ targetType: 'room' });
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.VIEW_LOGS, logs: [log] });
    expect(result.logs.room).toEqual([log]);
  });

  it('CLEAR_LOGS → resets logs to empty arrays', () => {
    const state = makeServerState({ logs: { room: [makeLogItem()], game: [], chat: [] } });
    const result = serverReducer(state, { type: Types.CLEAR_LOGS });
    expect(result.logs.room).toEqual([]);
    expect(result.logs.game).toEqual([]);
    expect(result.logs.chat).toEqual([]);
  });
});

// ── Messaging ─────────────────────────────────────────────────────────────────

describe('Messaging', () => {
  it('USER_MESSAGE → uses receiverName as key when current user is sender', () => {
    const state = makeServerState({ user: makeUser({ name: 'Alice' }), messages: {} });
    const messageData = { senderName: 'Alice', receiverName: 'Bob', message: 'hi' };
    const result = serverReducer(state, { type: Types.USER_MESSAGE, messageData });
    expect(result.messages['Bob']).toHaveLength(1);
    expect(result.messages['Bob'][0]).toBe(messageData);
  });

  it('USER_MESSAGE → uses senderName as key when current user is receiver', () => {
    const state = makeServerState({ user: makeUser({ name: 'Bob' }), messages: {} });
    const messageData = { senderName: 'Alice', receiverName: 'Bob', message: 'yo' };
    const result = serverReducer(state, { type: Types.USER_MESSAGE, messageData });
    expect(result.messages['Alice']).toHaveLength(1);
    expect(result.messages['Alice'][0]).toBe(messageData);
  });

  it('USER_MESSAGE → appends to existing messages for that user', () => {
    const existingMsg = create(Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'first' });
    const state = makeServerState({
      user: makeUser({ name: 'Bob' }),
      messages: { Alice: [existingMsg] },
    });
    const newMsg = create(Event_UserMessageSchema, { senderName: 'Alice', receiverName: 'Bob', message: 'second' });
    const result = serverReducer(state, { type: Types.USER_MESSAGE, messageData: newMsg });
    expect(result.messages['Alice']).toHaveLength(2);
  });
});

// ── User Info & Notifications ─────────────────────────────────────────────────

describe('User Info & Notifications', () => {
  it('GET_USER_INFO → adds userInfo keyed by name', () => {
    const userInfo = makeUser({ name: 'Eve' });
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.GET_USER_INFO, userInfo });
    expect(result.userInfo['Eve']).toBe(userInfo);
  });

  it('NOTIFY_USER → appends notification to list', () => {
    const state = makeServerState({ notifications: [] });
    const notification = { type: 1, warningReason: '', customTitle: '', customContent: '' };
    const result = serverReducer(state, { type: Types.NOTIFY_USER, notification });
    expect(result.notifications).toHaveLength(1);
    expect(result.notifications[0]).toBe(notification);
  });

  it('SERVER_SHUTDOWN → sets serverShutdown to action.data', () => {
    const data = { reason: 'maintenance', minutes: 10 };
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.SERVER_SHUTDOWN, data });
    expect(result.serverShutdown).toBe(data);
  });
});

// ── Moderation ────────────────────────────────────────────────────────────────

describe('Moderation', () => {
  it('BAN_FROM_SERVER → sets banUser', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.BAN_FROM_SERVER, userName: 'Frank' });
    expect(result.banUser).toBe('Frank');
  });

  it('BAN_HISTORY → adds banHistory keyed by userName', () => {
    const history = [makeBanHistoryItem()];
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.BAN_HISTORY, userName: 'Frank', banHistory: history });
    expect(result.banHistory['Frank']).toBe(history);
  });

  it('WARN_HISTORY → adds warnHistory keyed by userName', () => {
    const history = [makeWarnHistoryItem()];
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.WARN_HISTORY, userName: 'Grace', warnHistory: history });
    expect(result.warnHistory['Grace']).toBe(history);
  });

  it('WARN_LIST_OPTIONS → replaces warnListOptions', () => {
    const list = [makeWarnListItem()];
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.WARN_LIST_OPTIONS, warnList: list });
    expect(result.warnListOptions).toBe(list);
  });

  it('WARN_USER → sets warnUser', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.WARN_USER, userName: 'Hank' });
    expect(result.warnUser).toBe('Hank');
  });

  it('GET_ADMIN_NOTES → adds adminNotes keyed by userName', () => {
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.GET_ADMIN_NOTES, userName: 'Ira', notes: 'note1' });
    expect(result.adminNotes['Ira']).toBe('note1');
  });

  it('UPDATE_ADMIN_NOTES → updates adminNotes keyed by userName', () => {
    const state = makeServerState({ adminNotes: { Ira: 'old' } });
    const result = serverReducer(state, { type: Types.UPDATE_ADMIN_NOTES, userName: 'Ira', notes: 'new' });
    expect(result.adminNotes['Ira']).toBe('new');
  });
});

// ── ADJUST_MOD ────────────────────────────────────────────────────────────────

describe('ADJUST_MOD', () => {
  const baseUserLevel = UserLevelFlag.IsUser | UserLevelFlag.IsRegistered | UserLevelFlag.IsModerator | UserLevelFlag.IsJudge;

  it('shouldBeMod=true, shouldBeJudge=true → sets both bits, preserves IsUser|IsRegistered', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Dan', userLevel: baseUserLevel })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: true, shouldBeJudge: true });
    // IsUser(1) | IsRegistered(2) | IsModerator(4) | IsJudge(16) = 23
    expect(result.users[0].userLevel).toBe(23);
  });

  it('shouldBeMod=true, shouldBeJudge=false → sets IsModerator, clears IsJudge, preserves others', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Dan', userLevel: baseUserLevel })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: true, shouldBeJudge: false });
    // IsUser(1) | IsRegistered(2) | IsModerator(4) = 7
    expect(result.users[0].userLevel).toBe(7);
  });

  it('shouldBeMod=false, shouldBeJudge=true → clears IsModerator, sets IsJudge, preserves others', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Dan', userLevel: baseUserLevel })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: false, shouldBeJudge: true });
    // IsUser(1) | IsRegistered(2) | IsJudge(16) = 19
    expect(result.users[0].userLevel).toBe(19);
  });

  it('shouldBeMod=false, shouldBeJudge=false → clears both bits, preserves IsUser|IsRegistered', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Dan', userLevel: baseUserLevel })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: false, shouldBeJudge: false });
    // IsUser(1) | IsRegistered(2) = 3
    expect(result.users[0].userLevel).toBe(3);
  });

  it('shouldBeMod=true on IsUser|IsRegistered only → produces 7, not 4', () => {
    const state = makeServerState({ users: [makeUser({ name: 'Dan', userLevel: UserLevelFlag.IsUser | UserLevelFlag.IsRegistered })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: true, shouldBeJudge: false });
    // IsUser(1) | IsRegistered(2) | IsModerator(4) = 7
    expect(result.users[0].userLevel).toBe(7);
  });

  it('non-matching users are left unchanged', () => {
    const alice = makeUser({ name: 'Alice', userLevel: 7 });
    const state = makeServerState({ users: [alice, makeUser({ name: 'Dan', userLevel: baseUserLevel })] });
    const result = serverReducer(state, { type: Types.ADJUST_MOD, userName: 'Dan', shouldBeMod: false, shouldBeJudge: false });
    expect(result.users.find(u => u.name === 'Alice').userLevel).toBe(7);
  });
});

// ── Replays ───────────────────────────────────────────────────────────────────

describe('Replays', () => {
  it('REPLAY_LIST → replaces replays list', () => {
    const matchList = [makeReplayMatch({ gameId: 10 })];
    const state = makeServerState({ replays: [makeReplayMatch({ gameId: 99 })] });
    const result = serverReducer(state, { type: Types.REPLAY_LIST, matchList });
    expect(result.replays).toHaveLength(1);
    expect(result.replays[0].gameId).toBe(10);
  });

  it('REPLAY_ADDED → appends matchInfo to replays', () => {
    const existing = makeReplayMatch({ gameId: 1 });
    const added = makeReplayMatch({ gameId: 2 });
    const state = makeServerState({ replays: [existing] });
    const result = serverReducer(state, { type: Types.REPLAY_ADDED, matchInfo: added });
    expect(result.replays).toHaveLength(2);
    expect(result.replays[1]).toBe(added);
  });

  it('REPLAY_MODIFY_MATCH → updates doNotHide for matching gameId', () => {
    const state = makeServerState({ replays: [makeReplayMatch({ gameId: 5, doNotHide: false })] });
    const result = serverReducer(state, { type: Types.REPLAY_MODIFY_MATCH, gameId: 5, doNotHide: true });
    expect(result.replays[0].doNotHide).toBe(true);
  });

  it('REPLAY_MODIFY_MATCH → leaves non-matching replays unchanged', () => {
    const r1 = makeReplayMatch({ gameId: 1, doNotHide: false });
    const r2 = makeReplayMatch({ gameId: 2, doNotHide: false });
    const state = makeServerState({ replays: [r1, r2] });
    const result = serverReducer(state, { type: Types.REPLAY_MODIFY_MATCH, gameId: 1, doNotHide: true });
    expect(result.replays[1].doNotHide).toBe(false);
  });

  it('REPLAY_DELETE_MATCH → removes replay by gameId', () => {
    const state = makeServerState({ replays: [makeReplayMatch({ gameId: 5 }), makeReplayMatch({ gameId: 6 })] });
    const result = serverReducer(state, { type: Types.REPLAY_DELETE_MATCH, gameId: 5 });
    expect(result.replays).toHaveLength(1);
    expect(result.replays[0].gameId).toBe(6);
  });
});

// ── Deck Storage ──────────────────────────────────────────────────────────────

describe('Deck Storage', () => {
  it('BACKEND_DECKS → sets backendDecks', () => {
    const deckList = makeDeckList();
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.BACKEND_DECKS, deckList });
    expect(result.backendDecks).toBe(deckList);
  });

  it('DECK_UPLOAD with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, { type: Types.DECK_UPLOAD, path: '', treeItem: makeDeckTreeItem() });
    expect(result).toBe(state);
  });

  it('DECK_UPLOAD with flat path → appends item to root', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const item = makeDeckTreeItem({ name: 'deck.cod' });
    const result = serverReducer(state, { type: Types.DECK_UPLOAD, path: '', treeItem: item });
    expect(result.backendDecks.root.items).toHaveLength(1);
    expect(result.backendDecks.root.items[0]).toBe(item);
  });

  it('DECK_UPLOAD with nested path → inserts into matching subfolder', () => {
    const subfolder = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'myDecks', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const item = makeDeckTreeItem({ name: 'new.cod' });
    const result = serverReducer(state, { type: Types.DECK_UPLOAD, path: 'myDecks', treeItem: item });
    const folder = result.backendDecks.root.items.find(i => i.name === 'myDecks');
    expect(folder.folder.items).toHaveLength(1);
    expect(folder.folder.items[0]).toBe(item);
  });

  it('DECK_UPLOAD with non-existent intermediate folder → creates folder and inserts', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const item = makeDeckTreeItem({ name: 'deck.cod' });
    const result = serverReducer(state, { type: Types.DECK_UPLOAD, path: 'newFolder', treeItem: item });
    expect(result.backendDecks.root.items).toHaveLength(1);
    expect(result.backendDecks.root.items[0].name).toBe('newFolder');
    expect(result.backendDecks.root.items[0].folder.items[0]).toBe(item);
  });

  it('DECK_DELETE with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, { type: Types.DECK_DELETE, deckId: 1 });
    expect(result).toBe(state);
  });

  it('DECK_DELETE → removes item by id from tree', () => {
    const item = makeDeckTreeItem({ id: 7 });
    const state = makeServerState({ backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [item] }) }) });
    const result = serverReducer(state, { type: Types.DECK_DELETE, deckId: 7 });
    expect(result.backendDecks.root.items).toHaveLength(0);
  });

  it('DECK_DELETE → recursively removes item nested inside a subfolder', () => {
    const nested = makeDeckTreeItem({ id: 9, name: 'nested.cod' });
    const subfolder = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'sub', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [nested] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, { type: Types.DECK_DELETE, deckId: 9 });
    expect(result.backendDecks.root.items[0].folder.items).toHaveLength(0);
  });

  it('DECK_NEW_DIR with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, { type: Types.DECK_NEW_DIR, path: '', dirName: 'newDir' });
    expect(result).toBe(state);
  });

  it('DECK_NEW_DIR at root → appends folder to root items', () => {
    const state = makeServerState({ backendDecks: makeDeckList() });
    const result = serverReducer(state, { type: Types.DECK_NEW_DIR, path: '', dirName: 'myDir' });
    expect(result.backendDecks.root.items).toHaveLength(1);
    expect(result.backendDecks.root.items[0].name).toBe('myDir');
    expect(result.backendDecks.root.items[0].folder.items).toEqual([]);
  });

  it('DECK_NEW_DIR nested → inserts folder inside matching subfolder', () => {
    const subfolder = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'parent', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, { type: Types.DECK_NEW_DIR, path: 'parent', dirName: 'child' });
    const parent = result.backendDecks.root.items.find(i => i.name === 'parent');
    expect(parent.folder.items).toHaveLength(1);
    expect(parent.folder.items[0].name).toBe('child');
  });

  it('DECK_DEL_DIR with null backendDecks → returns state unchanged', () => {
    const state = makeServerState({ backendDecks: null });
    const result = serverReducer(state, { type: Types.DECK_DEL_DIR, path: 'myDir' });
    expect(result).toBe(state);
  });

  it('DECK_DEL_DIR → removes folder from root by name', () => {
    const subfolder = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'myDir', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, { type: Types.DECK_DEL_DIR, path: 'myDir' });
    expect(result.backendDecks.root.items).toHaveLength(0);
  });

  it('DECK_DEL_DIR → returns deck tree unchanged when path is empty', () => {
    const subfolder = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'keep', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [subfolder] }) })
    });
    const result = serverReducer(state, { type: Types.DECK_DEL_DIR, path: '' });
    expect(result.backendDecks.root.items).toHaveLength(1);
  });

  it('DECK_DEL_DIR → recursively removes nested subfolder via multi-segment path', () => {
    const child = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'child', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [] })
    });
    const parent = create(ServerInfo_DeckStorage_TreeItemSchema, {
      id: 0, name: 'parent', folder: create(ServerInfo_DeckStorage_FolderSchema, { items: [child] })
    });
    const state = makeServerState({
      backendDecks: makeDeckList({ root: create(ServerInfo_DeckStorage_FolderSchema, { items: [parent] }) })
    });
    const result = serverReducer(state, { type: Types.DECK_DEL_DIR, path: 'parent/child' });
    expect(result.backendDecks.root.items[0].folder.items).toHaveLength(0);
  });
});

// ── GAMES_OF_USER ─────────────────────────────────────────────────────────────

describe('GAMES_OF_USER', () => {
  it('stores normalized games keyed by userName', () => {
    const games = [makeGame({ gameId: 5 })];
    const state = makeServerState();
    const result = serverReducer(state, { type: Types.GAMES_OF_USER, userName: 'alice', games, gametypeMap: {} });
    expect(result.gamesOfUser['alice']).toEqual(games);
  });

  it('overwrites previous games for same user', () => {
    const old = [makeGame({ gameId: 1 })];
    const fresh = [makeGame({ gameId: 2 })];
    const state = makeServerState({ gamesOfUser: { alice: old } });
    const result = serverReducer(state, { type: Types.GAMES_OF_USER, userName: 'alice', games: fresh, gametypeMap: {} });
    expect(result.gamesOfUser['alice']).toEqual(fresh);
  });

  it('does not affect other users\' entries', () => {
    const bobGames = [makeGame({ gameId: 3 })];
    const state = makeServerState({ gamesOfUser: { bob: bobGames } });
    const result = serverReducer(state, { type: Types.GAMES_OF_USER, userName: 'alice', games: [], gametypeMap: {} });
    expect(result.gamesOfUser['bob']).toBe(bobGames);
  });
});
