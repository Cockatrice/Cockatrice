import { Selectors } from './rooms.selectors';
import { RoomsState } from './rooms.interfaces';
import { makeGame, makeMessage, makeRoom, makeRoomsState, makeUser } from './__mocks__/rooms-fixtures';
import { App } from '@app/types';

function rootState(rooms: RoomsState) {
  return { rooms };
}

describe('Selectors', () => {
  it('getRooms → returns rooms map', () => {
    const state = makeRoomsState();
    expect(Selectors.getRooms(rootState(state))).toBe(state.rooms);
  });

  it('getRoom → returns room matching roomId', () => {
    const room = makeRoom({ roomId: 1 });
    const state = makeRoomsState({ rooms: { 1: room } });
    expect(Selectors.getRoom(rootState(state), 1)).toBe(room);
  });

  it('getRoom → returns undefined for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    expect(Selectors.getRoom(rootState(state), 999)).toBeUndefined();
  });

  it('getJoinedRoomIds → returns joinedRoomIds', () => {
    const joinedRoomIds = { 1: true };
    const state = makeRoomsState({ joinedRoomIds });
    expect(Selectors.getJoinedRoomIds(rootState(state))).toBe(joinedRoomIds);
  });

  it('getJoinedGameIds → returns joinedGameIds', () => {
    const joinedGameIds = { 1: { 5: true } };
    const state = makeRoomsState({ joinedGameIds });
    expect(Selectors.getJoinedGameIds(rootState(state))).toBe(joinedGameIds);
  });

  it('getMessages → returns messages map', () => {
    const messages = { 1: [makeMessage()] };
    const state = makeRoomsState({ messages });
    expect(Selectors.getMessages(rootState(state))).toBe(messages);
  });

  it('getSortGamesBy → returns sortGamesBy', () => {
    const state = makeRoomsState();
    expect(Selectors.getSortGamesBy(rootState(state))).toBe(state.sortGamesBy);
  });

  it('getSortUsersBy → returns sortUsersBy', () => {
    const state = makeRoomsState();
    expect(Selectors.getSortUsersBy(rootState(state))).toBe(state.sortUsersBy);
  });

  it('getJoinedRooms → returns only rooms whose roomId is in joinedRoomIds', () => {
    const room1 = makeRoom({ roomId: 1 });
    const room2 = makeRoom({ roomId: 2 });
    const state = makeRoomsState({
      rooms: { 1: room1, 2: room2 },
      joinedRoomIds: { 1: true },
    });
    const result = Selectors.getJoinedRooms(rootState(state));
    expect(result).toHaveLength(1);
    expect(result[0]).toBe(room1);
  });

  it('getJoinedRooms → returns empty array when none joined', () => {
    const state = makeRoomsState({ rooms: { 1: makeRoom({ roomId: 1 }) }, joinedRoomIds: {} });
    expect(Selectors.getJoinedRooms(rootState(state))).toHaveLength(0);
  });

  it('getJoinedGames → returns only games whose gameId is in joinedGameIds for that room', () => {
    const game1 = makeGame({ gameId: 1 });
    const game2 = makeGame({ gameId: 2 });
    const room = makeRoom({ roomId: 1, games: { 1: game1, 2: game2 } });
    const state = makeRoomsState({
      rooms: { 1: room },
      joinedGameIds: { 1: { 1: true } },
    });
    const result = Selectors.getJoinedGames(rootState(state), 1);
    expect(result).toHaveLength(1);
    expect(result[0]).toBe(game1);
  });

  it('getJoinedGames → returns empty array when room is unknown', () => {
    const state = makeRoomsState({ rooms: {}, joinedGameIds: { 1: { 1: true } } });
    expect(Selectors.getJoinedGames(rootState(state), 1)).toHaveLength(0);
  });

  it('getRoomMessages → returns messages array for roomId', () => {
    const messages = [makeMessage()];
    const state = makeRoomsState({ messages: { 1: messages } });
    expect(Selectors.getRoomMessages(rootState(state), 1)).toBe(messages);
  });

  it('getRoomGames → returns keyed games map for roomId', () => {
    const game = makeGame({ gameId: 10 });
    const room = makeRoom({ roomId: 1, games: { 10: game } });
    const state = makeRoomsState({ rooms: { 1: room } });
    expect(Selectors.getRoomGames(rootState(state), 1)).toBe(room.games);
  });

  it('getRoomGames → returns EMPTY_GAMES_MAP for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    expect(Selectors.getRoomGames(rootState(state), 999)).toEqual({});
  });

  it('getRoomUsers → returns keyed users map for roomId', () => {
    const user = makeUser({ name: 'alice' });
    const room = makeRoom({ roomId: 1, users: { alice: user } });
    const state = makeRoomsState({ rooms: { 1: room } });
    expect(Selectors.getRoomUsers(rootState(state), 1)).toBe(room.users);
  });

  it('getSortedRoomGames → returns games sorted by the active sort config', () => {
    const game1 = makeGame({ gameId: 1, description: 'Beta' });
    const game2 = makeGame({ gameId: 2, description: 'Alpha' });
    const room = makeRoom({ roomId: 1, games: { 1: game1, 2: game2 } });
    const state = makeRoomsState({
      rooms: { 1: room },
      sortGamesBy: { field: 'info.description' as App.GameSortField, order: App.SortDirection.ASC },
    });
    const result = Selectors.getSortedRoomGames(rootState(state), 1);
    expect(result).toHaveLength(2);
    expect(result[0].info.description).toBe('Alpha');
    expect(result[1].info.description).toBe('Beta');
  });

  it('getSortedRoomGames → returns EMPTY_GAMES for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    expect(Selectors.getSortedRoomGames(rootState(state), 999)).toHaveLength(0);
  });

  it('getSortedRoomUsers → returns sorted user array sorted by name', () => {
    const zane = makeUser({ name: 'Zane' });
    const alice = makeUser({ name: 'Alice' });
    const room = makeRoom({ roomId: 1, users: { Zane: zane, Alice: alice } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const result = Selectors.getSortedRoomUsers(rootState(state), 1);
    expect(result[0].name).toBe('Alice');
    expect(result[1].name).toBe('Zane');
  });

  it('getSortedRoomUsers → returns EMPTY_USERS for unknown roomId', () => {
    const state = makeRoomsState({ rooms: {} });
    expect(Selectors.getSortedRoomUsers(rootState(state), 999)).toHaveLength(0);
  });


  it('getSortedRoomGames → returns same array reference for identical state', () => {
    const game = makeGame({ gameId: 1 });
    const room = makeRoom({ roomId: 1, games: { 1: game } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const root = rootState(state);
    const a = Selectors.getSortedRoomGames(root, 1);
    const b = Selectors.getSortedRoomGames(root, 1);
    expect(a).toBe(b);
  });

  it('getSortedRoomUsers → returns same array reference for identical state', () => {
    const user = makeUser({ name: 'Alice' });
    const room = makeRoom({ roomId: 1, users: { Alice: user } });
    const state = makeRoomsState({ rooms: { 1: room } });
    const root = rootState(state);
    const a = Selectors.getSortedRoomUsers(root, 1);
    const b = Selectors.getSortedRoomUsers(root, 1);
    expect(a).toBe(b);
  });

  it('getJoinedRooms → returns same array reference for identical state', () => {
    const room = makeRoom({ roomId: 1 });
    const state = makeRoomsState({ rooms: { 1: room }, joinedRoomIds: { 1: true } });
    const root = rootState(state);
    const a = Selectors.getJoinedRooms(root);
    const b = Selectors.getJoinedRooms(root);
    expect(a).toBe(b);
  });
});
