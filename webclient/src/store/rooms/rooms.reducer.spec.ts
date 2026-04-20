import { create } from '@bufbuild/protobuf';
import { App, Data } from '@app/types';
import { roomsReducer } from './rooms.reducer';
import { Actions } from './rooms.actions';
import { MAX_ROOM_MESSAGES } from './rooms.types';
import { DEFAULT_GAME_FILTERS } from './gameFilters';
import { makeGame, makeMessage, makeRoom, makeRoomsState, makeUser } from './__mocks__/rooms-fixtures';


describe('Initialisation', () => {
  it('returns initialState when called with undefined state', () => {
    const result = roomsReducer(undefined, { type: '@@INIT' });
    expect(result.rooms).toEqual({});
    expect(result.joinedRoomIds).toEqual({});
  });

  it('CLEAR_STORE → resets to initialState', () => {
    const state = makeRoomsState({ joinedRoomIds: { 1: true } });
    const result = roomsReducer(state, Actions.clearStore());
    expect(result.joinedRoomIds).toEqual({});
    expect(result.rooms).toEqual({});
  });

  it('default → returns state unchanged for unknown action', () => {
    const state = makeRoomsState();
    const result = roomsReducer(state, { type: '@@UNKNOWN' });
    expect(result).toEqual(state);
  });
});


describe('UPDATE_ROOMS', () => {
  it('creates RoomEntry with empty normalized games/users for new room', () => {
    const state = makeRoomsState({ rooms: {} });
    // UPDATE_ROOMS carries raw ServerInfo_Room protos via the action
    const room = makeRoom({ roomId: 1 }).info;
    const result = roomsReducer(state, Actions.updateRooms({ rooms: [room] }));
    expect(result.rooms[1]).toBeDefined();
    expect(result.rooms[1].info).toBe(room);
    expect(result.rooms[1].games).toEqual({});
    expect(result.rooms[1].users).toEqual({});
  });

  it('sets numeric order from array index', () => {
    const state = makeRoomsState({ rooms: {} });
    const rooms = [makeRoom({ roomId: 1 }).info, makeRoom({ roomId: 2 }).info];
    const result = roomsReducer(state, Actions.updateRooms({ rooms }));
    expect(result.rooms[1].order).toBe(0);
    expect(result.rooms[2].order).toBe(1);
  });

  it('preserves existing normalized games/users when merging into existing room', () => {
    const existingGame = makeGame({ gameId: 42 });
    const existingUser = makeUser({ name: 'alice' });
    const existingRoom = makeRoom({
      roomId: 1,
      name: 'Old Name',
      games: { 42: existingGame },
      users: { alice: existingUser },
    });
    const state = makeRoomsState({ rooms: { 1: existingRoom } });

    const update = makeRoom({ roomId: 1, name: 'New Name' }).info;
    const result = roomsReducer(state, Actions.updateRooms({ rooms: [update] }));

    expect(result.rooms[1].info.name).toBe('New Name');
    expect(result.rooms[1].games[42]).toBe(existingGame);
    expect(result.rooms[1].users['alice']).toBe(existingUser);
  });

  it('creates new room entry for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    const room = makeRoom({ roomId: 99, name: 'New Room' }).info;
    const result = roomsReducer(state, Actions.updateRooms({ rooms: [room] }));
    expect(result.rooms[99]).toBeDefined();
    expect(result.rooms[99].info.name).toBe('New Room');
  });

  it('partial room update (only playerCount) preserves name, description, and gametypeMap', () => {
    // Regression: the desktop server fires Event_ListRooms with only
    // room_id/player_count/game_count set on every user join/leave
    // (server_room.cpp addClient/removeClient). A wholesale info replacement
    // would blank out name/description until the next full room listing.
    const gametypeMap = { 0: 'Constructed' };
    const existingRoom = makeRoom({
      roomId: 1,
      name: 'Main Hall',
      description: 'General play',
      permissionlevel: 'none',
      gametypeMap,
    });
    const state = makeRoomsState({ rooms: { 1: existingRoom } });

    const partial = create(Data.ServerInfo_RoomSchema, {
      roomId: 1,
      playerCount: 42,
      gameCount: 3,
    });
    const result = roomsReducer(state, Actions.updateRooms({ rooms: [partial] }));

    expect(result.rooms[1].info.name).toBe('Main Hall');
    expect(result.rooms[1].info.description).toBe('General play');
    expect(result.rooms[1].info.permissionlevel).toBe('none');
    expect(result.rooms[1].info.playerCount).toBe(42);
    expect(result.rooms[1].info.gameCount).toBe(3);
    expect(result.rooms[1].gametypeMap).toBe(gametypeMap);
  });
});


describe('JOIN_ROOM', () => {
  it('normalizes raw room into keyed games/users maps and marks joined', () => {
    const state = makeRoomsState({ rooms: {}, joinedRoomIds: {} });
    // JOIN_ROOM carries a raw proto Room with its gameList/userList populated
    const rawRoom = makeRoom({
      roomId: 2,
      gameList: [makeGame({ gameId: 1 }).info],
      userList: [makeUser({ name: 'Zane' }), makeUser({ name: 'Alice' })],
    }).info;
    const result = roomsReducer(state, Actions.joinRoom({ roomInfo: rawRoom }));
    expect(result.joinedRoomIds[2]).toBe(true);
    expect(result.rooms[2].users['Alice']).toBeDefined();
    expect(result.rooms[2].users['Zane']).toBeDefined();
    expect(result.rooms[2].games[1]).toBeDefined();
    expect(result.rooms[2].info.roomId).toBe(2);
  });
});


describe('LEAVE_ROOM', () => {
  it('removes joinedRoomIds entry and messages for roomId', () => {
    const state = makeRoomsState({
      joinedRoomIds: { 1: true },
      messages: { 1: [makeMessage()] },
    });
    const result = roomsReducer(state, Actions.leaveRoom({ roomId: 1 }));
    expect(result.joinedRoomIds[1]).toBeUndefined();
    expect(result.messages[1]).toBeUndefined();
  });
});


describe('ADD_MESSAGE', () => {
  it('appends message preserving the timeReceived from the event handler', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ message: 'hello', timeReceived: 1700000000000 });
    const result = roomsReducer(state, Actions.addMessage({ roomId: 1, message }));
    expect(result.messages[1]).toHaveLength(1);
    expect(result.messages[1][0].timeReceived).toBe(1700000000000);
  });

  it('creates message list for roomId when none exists', () => {
    const state = makeRoomsState({ messages: {} });
    const message = makeMessage();
    const result = roomsReducer(state, Actions.addMessage({ roomId: 5, message }));
    expect(result.messages[5]).toHaveLength(1);
  });

  it(`shifts oldest message when list is at MAX_ROOM_MESSAGES (${MAX_ROOM_MESSAGES})`, () => {
    const firstMsg = makeMessage({ message: 'first' });
    const messages = Array.from({ length: MAX_ROOM_MESSAGES }, (_, i) =>
      i === 0 ? firstMsg : makeMessage({ message: `msg-${i}` })
    );
    const state = makeRoomsState({ messages: { 1: messages } });
    const newMsg = makeMessage({ message: 'new' });
    const result = roomsReducer(state, Actions.addMessage({ roomId: 1, message: newMsg }));
    expect(result.messages[1]).toHaveLength(MAX_ROOM_MESSAGES);
    expect(result.messages[1][0].message).not.toBe('first');
    expect(result.messages[1][MAX_ROOM_MESSAGES - 1].message).toBe('new');
  });

  it('prepends "name: " to message when name is present', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ name: 'Alice', message: 'hello' });
    const result = roomsReducer(state, Actions.addMessage({ roomId: 1, message }));
    expect(result.messages[1][0].message).toBe('Alice: hello');
  });

  it('does not prepend when name is empty', () => {
    const state = makeRoomsState({ messages: { 1: [] } });
    const message = makeMessage({ name: '', message: 'system msg' });
    const result = roomsReducer(state, Actions.addMessage({ roomId: 1, message }));
    expect(result.messages[1][0].message).toBe('system msg');
  });
});


describe('UPDATE_GAMES', () => {
  it('removes closed games from the keyed games map', () => {
    const room = makeRoom({ roomId: 1, games: { 1: makeGame({ gameId: 1 }) } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, Actions.updateGames({
      roomId: 1,
      games: [{ gameId: 1, closed: true }],
    }));
    expect(result.rooms[1].games[1]).toBeUndefined();
  });

  it('merges update into existing game info', () => {
    const game = makeGame({ gameId: 1, description: 'old' });
    const room = makeRoom({ roomId: 1, games: { 1: game } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, Actions.updateGames({
      roomId: 1,
      games: [{ gameId: 1, description: 'new' }],
    }));
    expect(result.rooms[1].games[1].info.description).toBe('new');
  });

  it('inserts new game into the keyed map', () => {
    const room = makeRoom({ roomId: 1 });
    const state = makeRoomsState({ rooms: { 1: room } });
    const newGame = makeGame({ gameId: 99, description: 'extra' }).info;
    const result = roomsReducer(state, Actions.updateGames({ roomId: 1, games: [newGame] }));
    expect(Object.keys(result.rooms[1].games)).toHaveLength(1);
    expect(result.rooms[1].games[99]).toBeDefined();
    expect(result.rooms[1].games[99].info.gameId).toBe(99);
  });

  it('preserves existing games not included in the update', () => {
    const game1 = makeGame({ gameId: 1, description: 'untouched' });
    const game2 = makeGame({ gameId: 2, description: 'old' });
    const room = makeRoom({ roomId: 1, games: { 1: game1, 2: game2 } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, Actions.updateGames({
      roomId: 1,
      games: [{ gameId: 2, description: 'new' }],
    }));
    expect(result.rooms[1].games[1].info.description).toBe('untouched');
    expect(result.rooms[1].games[2].info.description).toBe('new');
  });

  it('returns state identity when roomId is unknown', () => {
    const state = makeRoomsState({ rooms: {} });
    const result = roomsReducer(state, Actions.updateGames({ roomId: 999, games: [] }));
    expect(result).toEqual(state);
  });
});


describe('USER_JOINED', () => {
  it('inserts user into the keyed users map', () => {
    const room = makeRoom({ roomId: 1, users: { Zane: makeUser({ name: 'Zane' }) } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, Actions.userJoined({ roomId: 1, user: makeUser({ name: 'Alice' }) }));
    expect(result.rooms[1].users['Alice']).toBeDefined();
    expect(result.rooms[1].users['Zane']).toBeDefined();
    expect(Object.keys(result.rooms[1].users)).toHaveLength(2);
  });
});

describe('USER_LEFT', () => {
  it('removes user by name from the keyed users map', () => {
    const room = makeRoom({
      roomId: 1,
      users: { Alice: makeUser({ name: 'Alice' }), Bob: makeUser({ name: 'Bob' }) },
    });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = roomsReducer(state, Actions.userLeft({ roomId: 1, name: 'Alice' }));
    expect(result.rooms[1].users['Alice']).toBeUndefined();
    expect(result.rooms[1].users['Bob']).toBeDefined();
  });
});


describe('SORT_GAMES', () => {
  it('updates sortGamesBy on state (sorting itself is now derived in selectors)', () => {
    const state = makeRoomsState({ rooms: {} });
    const result = roomsReducer(state, Actions.sortGames({
      roomId: 1,
      field: App.GameSortField.START_TIME,
      order: App.SortDirection.ASC,
    }));
    expect(result.sortGamesBy).toEqual({ field: App.GameSortField.START_TIME, order: App.SortDirection.ASC });
  });
});


describe('REMOVE_MESSAGES', () => {
  it('removes messages starting with "name:" up to amount, in reverse scan order', () => {
    const msgs = [
      makeMessage({ message: 'Alice: hello' }),
      makeMessage({ message: 'Bob: hi' }),
      makeMessage({ message: 'Alice: world' }),
    ];
    const state = makeRoomsState({ messages: { 1: msgs } });
    const result = roomsReducer(state, Actions.removeMessages({ roomId: 1, name: 'Alice', amount: 1 }));
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
    const result = roomsReducer(state, Actions.removeMessages({ roomId: 1, name: 'Alice', amount: 2 }));
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
    const result = roomsReducer(state, Actions.removeMessages({ roomId: 1, name: 'Alice', amount: 1 }));
    expect(result.messages[1]).toHaveLength(2);
  });
});


describe('GAME_CREATED', () => {
  it('returns state unchanged', () => {
    const state = makeRoomsState();
    const result = roomsReducer(state, Actions.gameCreated({ roomId: 1 }));
    expect(result).toEqual(state);
  });
});


describe('JOINED_GAME', () => {
  it('sets joinedGameIds[roomId][gameId] = true', () => {
    const state = makeRoomsState({ joinedGameIds: {} });
    const result = roomsReducer(state, Actions.joinedGame({ roomId: 1, gameId: 5 }));
    expect(result.joinedGameIds[1][5]).toBe(true);
  });

  it('preserves other roomId entries in joinedGameIds', () => {
    const state = makeRoomsState({ joinedGameIds: { 2: { 9: true } } });
    const result = roomsReducer(state, Actions.joinedGame({ roomId: 1, gameId: 5 }));
    expect(result.joinedGameIds[2][9]).toBe(true);
    expect(result.joinedGameIds[1][5]).toBe(true);
  });
});


describe('SELECT_GAME', () => {
  it('records the selected gameId for the room', () => {
    const state = makeRoomsState();
    const result = roomsReducer(state, Actions.selectGame({ roomId: 1, gameId: 7 }));
    expect(result.selectedGameIds[1]).toBe(7);
  });

  it('clears the selection when called with undefined', () => {
    const state = makeRoomsState({ selectedGameIds: { 1: 7 } });
    const result = roomsReducer(state, Actions.selectGame({ roomId: 1, gameId: undefined }));
    expect(result.selectedGameIds[1]).toBeUndefined();
  });

  it('keeps other rooms\u2019 selections untouched', () => {
    const state = makeRoomsState({ selectedGameIds: { 1: 7, 2: 11 } });
    const result = roomsReducer(state, Actions.selectGame({ roomId: 1, gameId: 8 }));
    expect(result.selectedGameIds[2]).toBe(11);
  });
});


describe('UPDATE_GAMES \u2014 selection lifecycle', () => {
  it('clears selectedGameIds[roomId] when the selected game is closed', () => {
    const room = makeRoom({ roomId: 1, games: { 5: makeGame({ gameId: 5 }) } });
    const state = makeRoomsState({ rooms: { 1: room }, selectedGameIds: { 1: 5 } });
    const result = roomsReducer(state, Actions.updateGames({
      roomId: 1,
      games: [{ gameId: 5, closed: true } as any],
    }));
    expect(result.selectedGameIds[1]).toBeUndefined();
  });

  it('preserves selection when an unrelated game is closed', () => {
    const room = makeRoom({ roomId: 1, games: { 5: makeGame({ gameId: 5 }), 6: makeGame({ gameId: 6 }) } });
    const state = makeRoomsState({ rooms: { 1: room }, selectedGameIds: { 1: 5 } });
    const result = roomsReducer(state, Actions.updateGames({
      roomId: 1,
      games: [{ gameId: 6, closed: true } as any],
    }));
    expect(result.selectedGameIds[1]).toBe(5);
  });
});


describe('LEAVE_ROOM \u2014 selection and filters', () => {
  it('clears selectedGameIds and gameFilters for the leaving room', () => {
    const state = makeRoomsState({
      selectedGameIds: { 1: 5, 2: 7 },
      gameFilters: { 1: { ...DEFAULT_GAME_FILTERS, hideFullGames: true } },
    });
    const result = roomsReducer(state, Actions.leaveRoom({ roomId: 1 }));
    expect(result.selectedGameIds[1]).toBeUndefined();
    expect(result.selectedGameIds[2]).toBe(7);
    expect(result.gameFilters[1]).toBeUndefined();
  });
});


describe('SET_GAME_FILTERS / CLEAR_GAME_FILTERS', () => {
  it('SET_GAME_FILTERS stores filter state for the room', () => {
    const state = makeRoomsState();
    const filters = { ...DEFAULT_GAME_FILTERS, hideFullGames: true };
    const result = roomsReducer(state, Actions.setGameFilters({ roomId: 1, filters }));
    expect(result.gameFilters[1]).toEqual(filters);
  });

  it('CLEAR_GAME_FILTERS resets the room filter state to defaults', () => {
    const state = makeRoomsState({
      gameFilters: { 1: { ...DEFAULT_GAME_FILTERS, hideFullGames: true } },
    });
    const result = roomsReducer(state, Actions.clearGameFilters({ roomId: 1 }));
    expect(result.gameFilters[1]).toEqual(DEFAULT_GAME_FILTERS);
  });
});
