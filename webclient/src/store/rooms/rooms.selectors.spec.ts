import { Selectors } from './rooms.selectors';
import { RoomsState } from './rooms.interfaces';
import { DEFAULT_GAME_FILTERS } from './gameFilters';
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

  describe('getSelectedGameId', () => {
    it('returns the selected gameId for the room', () => {
      const state = makeRoomsState({ selectedGameIds: { 1: 5 } });
      expect(Selectors.getSelectedGameId(rootState(state), 1)).toBe(5);
    });

    it('returns undefined when no game is selected', () => {
      const state = makeRoomsState();
      expect(Selectors.getSelectedGameId(rootState(state), 1)).toBeUndefined();
    });
  });

  describe('getGameFilters', () => {
    it('returns DEFAULT_GAME_FILTERS when room has no filter state', () => {
      const state = makeRoomsState();
      expect(Selectors.getGameFilters(rootState(state), 1)).toBe(DEFAULT_GAME_FILTERS);
    });

    it('returns the stored filters when set', () => {
      const filters = { ...DEFAULT_GAME_FILTERS, hideFullGames: true };
      const state = makeRoomsState({ gameFilters: { 1: filters } });
      expect(Selectors.getGameFilters(rootState(state), 1)).toBe(filters);
    });
  });

  describe('isGameFilterActive', () => {
    it('returns false when room has no filter state', () => {
      const state = makeRoomsState();
      expect(Selectors.isGameFilterActive(rootState(state), 1)).toBe(false);
    });

    it('returns false when filter state matches defaults', () => {
      const state = makeRoomsState({ gameFilters: { 1: { ...DEFAULT_GAME_FILTERS } } });
      expect(Selectors.isGameFilterActive(rootState(state), 1)).toBe(false);
    });

    it('returns true when any filter differs from defaults', () => {
      const state = makeRoomsState({ gameFilters: { 1: { ...DEFAULT_GAME_FILTERS, hideFullGames: true } } });
      expect(Selectors.isGameFilterActive(rootState(state), 1)).toBe(true);
    });
  });

  describe('getFilteredRoomGames', () => {
    it('returns the sorted list unchanged when no filters are set', () => {
      const game1 = makeGame({ gameId: 1, description: 'Beta' });
      const game2 = makeGame({ gameId: 2, description: 'Alpha' });
      const room = makeRoom({ roomId: 1, games: { 1: game1, 2: game2 } });
      const state = makeRoomsState({
        rooms: { 1: room },
        sortGamesBy: { field: 'info.description' as App.GameSortField, order: App.SortDirection.ASC },
      });
      const result = Selectors.getFilteredRoomGames(rootState(state), 1);
      expect(result).toHaveLength(2);
      expect(result[0].info.description).toBe('Alpha');
    });

    it('applies the active filter against the sorted list', () => {
      const open = makeGame({ gameId: 1, description: 'open', playerCount: 1, maxPlayers: 4 });
      const full = makeGame({ gameId: 2, description: 'full', playerCount: 4, maxPlayers: 4 });
      const room = makeRoom({ roomId: 1, games: { 1: open, 2: full } });
      const state = makeRoomsState({
        rooms: { 1: room },
        gameFilters: { 1: { ...DEFAULT_GAME_FILTERS, hideFullGames: true } },
      });
      const result = Selectors.getFilteredRoomGames(rootState(state), 1);
      expect(result).toHaveLength(1);
      expect(result[0].info.gameId).toBe(1);
    });

    it('returns EMPTY_GAMES for unknown roomId', () => {
      const state = makeRoomsState({ rooms: {} });
      expect(Selectors.getFilteredRoomGames(rootState(state), 999)).toHaveLength(0);
    });
  });

  describe('getRoomGameCounts', () => {
    it('reports total = visible when no filters are active', () => {
      const room = makeRoom({ roomId: 1, games: { 1: makeGame({ gameId: 1 }), 2: makeGame({ gameId: 2 }) } });
      const state = makeRoomsState({ rooms: { 1: room } });
      expect(Selectors.getRoomGameCounts(rootState(state), 1)).toEqual({ visible: 2, total: 2 });
    });

    it('reports a smaller visible count when filters hide games', () => {
      const open = makeGame({ gameId: 1, playerCount: 1, maxPlayers: 4 });
      const full = makeGame({ gameId: 2, playerCount: 4, maxPlayers: 4 });
      const room = makeRoom({ roomId: 1, games: { 1: open, 2: full } });
      const state = makeRoomsState({
        rooms: { 1: room },
        gameFilters: { 1: { ...DEFAULT_GAME_FILTERS, hideFullGames: true } },
      });
      expect(Selectors.getRoomGameCounts(rootState(state), 1)).toEqual({ visible: 1, total: 2 });
    });

    it('returns 0/0 for unknown roomId', () => {
      const state = makeRoomsState({ rooms: {} });
      expect(Selectors.getRoomGameCounts(rootState(state), 999)).toEqual({ visible: 0, total: 0 });
    });
  });
});
