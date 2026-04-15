import { App } from '@app/types';
import { roomsReducer } from './rooms.reducer';
import { Types, MAX_ROOM_MESSAGES } from './rooms.types';
import { makeGame, makeMessage, makeRoom, makeRoomsState, makeUser } from './__mocks__/rooms-fixtures';

// ── Initialisation ───────────────────────────────────────────────────────────

describe('Initialisation', () => {
  it('returns initialState when called with undefined state', () => {
    const result = roomsReducer(undefined, { type: '@@INIT' });
    expect(result.rooms).toEqual({});
    expect(result.joinedRoomIds).toEqual({});
  });

  it('CLEAR_STORE → resets to initialState', () => {
    const state = makeRoomsState({ joinedRoomIds: { 1: true } });
    const result = roomsReducer(state, { type: Types.CLEAR_STORE });
    expect(result.joinedRoomIds).toEqual({});
    expect(result.rooms).toEqual({});
  });

  it('default → returns state unchanged for unknown action', () => {
    const state = makeRoomsState();
    const result = roomsReducer(state, { type: '@@UNKNOWN' });
    expect(result).toBe(state);
  });
});

// ── UPDATE_ROOMS ──────────────────────────────────────────────────────────────

describe('UPDATE_ROOMS', () => {
  it('merges rooms and strips gameList, gametypeList, userList from update', () => {
    const state = makeRoomsState({ rooms: {} });
    const room = { ...makeRoom({ roomId: 1 }), gameList: [makeGame()], userList: [makeUser()], gametypeList: ['standard'] };
    const result = roomsReducer(state, { type: Types.UPDATE_ROOMS, rooms: [room] });
    expect(result.rooms[1]).toBeDefined();
    expect(result.rooms[1].gameList).toBeUndefined();
    expect(result.rooms[1].userList).toBeUndefined();
    expect(result.rooms[1].gametypeList).toBeUndefined();
  });

  it('sets numeric order from array index', () => {
    const state = makeRoomsState({ rooms: {} });
    const rooms = [makeRoom({ roomId: 1 }), makeRoom({ roomId: 2 })];
    const result = roomsReducer(state, { type: Types.UPDATE_ROOMS, rooms });
    expect(result.rooms[1].order).toBe(0);
    expect(result.rooms[2].order).toBe(1);
  });

  it('merges into existing room entry (preserves existing fields)', () => {
    const existingRoom = makeRoom({ roomId: 1, name: 'Old Name', gameList: [makeGame()] });
    const state = makeRoomsState({ rooms: { 1: existingRoom } });
    const update = makeRoom({ roomId: 1, name: 'New Name' });
    const result = roomsReducer(state, { type: Types.UPDATE_ROOMS, rooms: [update] });
    expect(result.rooms[1].name).toBe('New Name');
    expect(result.rooms[1].gameList).toEqual([makeGame()]);
  });

  it('creates new room entry for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    const room = makeRoom({ roomId: 99, name: 'New Room' });
    const result = roomsReducer(state, { type: Types.UPDATE_ROOMS, rooms: [room] });
    expect(result.rooms[99]).toBeDefined();
    expect(result.rooms[99].name).toBe('New Room');
  });
});

// ── JOIN_ROOM ──────────────────────────────────────────────────────────────────

describe('JOIN_ROOM', () => {
  it('copies gameList and userList, sorts both, sets joinedRoomIds', () => {
    const state = makeRoomsState({ rooms: {}, joinedRoomIds: {} });
    const roomInfo = makeRoom({
      roomId: 2,
      gameList: [makeGame({ gameId: 1 })],
      userList: [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })],
    });
    const result = roomsReducer(state, { type: Types.JOIN_ROOM, roomInfo });
    expect(result.joinedRoomIds[2]).toBe(true);
    expect(result.rooms[2].userList[0].name).toBe('Alice');
    expect(result.rooms[2]).toMatchObject({ roomId: 2 });
  });
});

// ── LEAVE_ROOM ────────────────────────────────────────────────────────────────

describe('LEAVE_ROOM', () => {
  it('removes joinedRoomIds entry and messages for roomId', () => {
    const state = makeRoomsState({
      joinedRoomIds: { 1: true },
      messages: { 1: [makeMessage()] },
    });
    const result = roomsReducer(state, { type: Types.LEAVE_ROOM, roomId: 1 });
    expect(result.joinedRoomIds[1]).toBeUndefined();
    expect(result.messages[1]).toBeUndefined();
  });
});

// ── ADD_MESSAGE ───────────────────────────────────────────────────────────────

describe('ADD_MESSAGE', () => {
  it('appends message with timeReceived set', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ message: 'hello', timeReceived: 0 });
    const result = roomsReducer(state, { type: Types.ADD_MESSAGE, roomId: 1, message });
    expect(result.messages[1]).toHaveLength(1);
    expect(result.messages[1][0].timeReceived).toBeGreaterThan(0);
  });

  it('creates message list for roomId when none exists', () => {
    const state = makeRoomsState({ messages: {} });
    const message = makeMessage();
    const result = roomsReducer(state, { type: Types.ADD_MESSAGE, roomId: 5, message });
    expect(result.messages[5]).toHaveLength(1);
  });

  it(`shifts oldest message when list is at MAX_ROOM_MESSAGES (${MAX_ROOM_MESSAGES})`, () => {
    const firstMsg = makeMessage({ message: 'first' });
    const messages = Array.from({ length: MAX_ROOM_MESSAGES }, (_, i) =>
      i === 0 ? firstMsg : makeMessage({ message: `msg-${i}` })
    );
    const state = makeRoomsState({ messages: { 1: messages } });
    const newMsg = makeMessage({ message: 'new' });
    const result = roomsReducer(state, { type: Types.ADD_MESSAGE, roomId: 1, message: newMsg });
    expect(result.messages[1]).toHaveLength(MAX_ROOM_MESSAGES);
    expect(result.messages[1][0].message).not.toBe('first');
    expect(result.messages[1][MAX_ROOM_MESSAGES - 1].message).toBe('new');
  });

  it('prepends "name: " to message when name is present', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ name: 'Alice', message: 'hello' });
    const result = roomsReducer(state, { type: Types.ADD_MESSAGE, roomId: 1, message });
    expect(result.messages[1][0].message).toBe('Alice: hello');
  });

  it('does not prepend when name is empty', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ name: '', message: 'system msg' });
    const result = roomsReducer(state, { type: Types.ADD_MESSAGE, roomId: 1, message });
    expect(result.messages[1][0].message).toBe('system msg');
  });
});

// ── UPDATE_GAMES ──────────────────────────────────────────────────────────────

describe('UPDATE_GAMES', () => {
  it('removes closed games from gameList', () => {
    const room = makeRoom({ roomId: 1, gameList: [makeGame({ gameId: 1 })] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, {
      type: Types.UPDATE_GAMES,
      roomId: 1,
      games: [{ gameId: 1, closed: true }],
    });
    expect(result.rooms[1].gameList).toHaveLength(0);
  });

  it('merges update into existing game', () => {
    const game = makeGame({ gameId: 1, description: 'old' });
    const room = makeRoom({ roomId: 1, gameList: [game] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, {
      type: Types.UPDATE_GAMES,
      roomId: 1,
      games: [{ gameId: 1, description: 'new' }],
    });
    expect(result.rooms[1].gameList[0].description).toBe('new');
  });

  it('appends new game to list and sorts', () => {
    const room = makeRoom({ roomId: 1, gameList: [] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const newGame = makeGame({ gameId: 99, description: 'extra' });
    const result = roomsReducer(state, { type: Types.UPDATE_GAMES, roomId: 1, games: [newGame] });
    expect(result.rooms[1].gameList).toHaveLength(1);
    expect(result.rooms[1].gameList[0].gameId).toBe(99);
  });

  it('preserves existing games not included in the update', () => {
    const game1 = makeGame({ gameId: 1, description: 'untouched' });
    const game2 = makeGame({ gameId: 2, description: 'old' });
    const room = makeRoom({ roomId: 1, gameList: [game1, game2] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, {
      type: Types.UPDATE_GAMES,
      roomId: 1,
      games: [{ gameId: 2, description: 'new' }],
    });
    expect(result.rooms[1].gameList.find(g => g.gameId === 1).description).toBe('untouched');
    expect(result.rooms[1].gameList.find(g => g.gameId === 2).description).toBe('new');
  });

  it('returns state identity when roomId is unknown', () => {
    const state = makeRoomsState({ rooms: {} });
    const result = roomsReducer(state, { type: Types.UPDATE_GAMES, roomId: 999, games: [] });
    expect(result).toBe(state);
  });
});

// ── USER_JOINED / USER_LEFT ───────────────────────────────────────────────────

describe('USER_JOINED', () => {
  it('appends user to userList and sorts by name ASC', () => {
    const room = makeRoom({ roomId: 1, userList: [makeUser({ name: 'Zane' })] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, { type: Types.USER_JOINED, roomId: 1, user: makeUser({ name: 'Alice' }) });
    expect(result.rooms[1].userList[0].name).toBe('Alice');
    expect(result.rooms[1].userList).toHaveLength(2);
  });
});

describe('USER_LEFT', () => {
  it('removes user by name from userList', () => {
    const room = makeRoom({ roomId: 1, userList: [makeUser({ name: 'Alice' }), makeUser({ name: 'Bob' })] });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, { type: Types.USER_LEFT, roomId: 1, name: 'Alice' });
    expect(result.rooms[1].userList).toHaveLength(1);
    expect(result.rooms[1].userList[0].name).toBe('Bob');
  });
});

// ── SORT_GAMES ────────────────────────────────────────────────────────────────

describe('SORT_GAMES', () => {
  it('resorts gameList and updates sortGamesBy on state', () => {
    const games = [makeGame({ gameId: 2 }), makeGame({ gameId: 1 })];
    const room = makeRoom({ roomId: 1, gameList: games });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, {
      type: Types.SORT_GAMES,
      roomId: 1,
      field: App.GameSortField.START_TIME,
      order: App.SortDirection.ASC,
    });
    expect(result.sortGamesBy).toEqual({ field: App.GameSortField.START_TIME, order: App.SortDirection.ASC });
  });
});

// ── REMOVE_MESSAGES ───────────────────────────────────────────────────────────

describe('REMOVE_MESSAGES', () => {
  it('removes messages starting with "name:" up to amount, in reverse scan order', () => {
    const msgs = [
      makeMessage({ message: 'Alice: hello' }),
      makeMessage({ message: 'Bob: hi' }),
      makeMessage({ message: 'Alice: world' }),
    ];
    const state = makeRoomsState({ messages: { 1: msgs } });
    const result = roomsReducer(state, { type: Types.REMOVE_MESSAGES, roomId: 1, name: 'Alice', amount: 1 });
    // reverse scan: removes LAST 'Alice:' message first, stops after 1
    const remaining = result.messages[1];
    expect(remaining).toHaveLength(2);
    const aliceMessages = remaining.filter(m => m.message.startsWith('Alice:'));
    expect(aliceMessages).toHaveLength(1);
    expect(aliceMessages[0].message).toBe('Alice: hello');
  });

  it('removes up to amount matching messages', () => {
    const msgs = [
      makeMessage({ message: 'Alice: one' }),
      makeMessage({ message: 'Alice: two' }),
      makeMessage({ message: 'Alice: three' }),
    ];
    const state = makeRoomsState({ messages: { 1: msgs } });
    const result = roomsReducer(state, { type: Types.REMOVE_MESSAGES, roomId: 1, name: 'Alice', amount: 2 });
    const remaining = result.messages[1];
    expect(remaining).toHaveLength(1);
  });

  it('stops removing once amount is reached', () => {
    const msgs = [
      makeMessage({ message: 'Alice: a' }),
      makeMessage({ message: 'Alice: b' }),
      makeMessage({ message: 'Alice: c' }),
    ];
    const state = makeRoomsState({ messages: { 1: msgs } });
    const result = roomsReducer(state, { type: Types.REMOVE_MESSAGES, roomId: 1, name: 'Alice', amount: 1 });
    expect(result.messages[1]).toHaveLength(2);
  });
});

// ── GAME_CREATED ──────────────────────────────────────────────────────────────

describe('GAME_CREATED', () => {
  it('returns state unchanged', () => {
    const state = makeRoomsState();
    const result = roomsReducer(state, { type: Types.GAME_CREATED, roomId: 1 });
    expect(result).toBe(state);
  });
});

// ── JOINED_GAME ───────────────────────────────────────────────────────────────

describe('JOINED_GAME', () => {
  it('sets joinedGameIds[roomId][gameId] = true', () => {
    const state = makeRoomsState({ joinedGameIds: {} });
    const result = roomsReducer(state, { type: Types.JOINED_GAME, roomId: 1, gameId: 5 });
    expect(result.joinedGameIds[1][5]).toBe(true);
  });

  it('preserves other roomId entries in joinedGameIds', () => {
    const state = makeRoomsState({ joinedGameIds: { 2: { 9: true } } });
    const result = roomsReducer(state, { type: Types.JOINED_GAME, roomId: 1, gameId: 5 });
    expect(result.joinedGameIds[2][9]).toBe(true);
    expect(result.joinedGameIds[1][5]).toBe(true);
  });
});
