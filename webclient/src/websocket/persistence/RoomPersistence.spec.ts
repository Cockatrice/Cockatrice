vi.mock('store', () => ({
  store: { getState: vi.fn().mockReturnValue({}) },
  RoomsDispatch: {
    clearStore: vi.fn(),
    joinRoom: vi.fn(),
    leaveRoom: vi.fn(),
    updateRooms: vi.fn(),
    updateGames: vi.fn(),
    addMessage: vi.fn(),
    userJoined: vi.fn(),
    userLeft: vi.fn(),
    removeMessages: vi.fn(),
    gameCreated: vi.fn(),
    joinedGame: vi.fn(),
  },
  RoomsSelectors: {
    getRoom: vi.fn(),
  },
}));

vi.mock('../utils/NormalizeService', () => ({
  __esModule: true,
  default: {
    normalizeRoomInfo: vi.fn(),
    normalizeGameObject: vi.fn(),
    normalizeUserMessage: vi.fn(),
  },
}));

import { RoomPersistence } from './RoomPersistence';
import { store, RoomsDispatch, RoomsSelectors } from 'store';
import NormalizeService from '../utils/NormalizeService';

beforeEach(() => {
  vi.clearAllMocks();
});

describe('RoomPersistence', () => {
  it('clearStore -> RoomsDispatch.clearStore', () => {
    RoomPersistence.clearStore();
    expect(RoomsDispatch.clearStore).toHaveBeenCalled();
  });

  it('joinRoom normalizes and dispatches', () => {
    const room = { roomId: 1 } as any;
    RoomPersistence.joinRoom(room);
    expect(NormalizeService.normalizeRoomInfo).toHaveBeenCalledWith(room);
    expect(RoomsDispatch.joinRoom).toHaveBeenCalledWith(room);
  });

  it('leaveRoom -> RoomsDispatch.leaveRoom', () => {
    RoomPersistence.leaveRoom(5);
    expect(RoomsDispatch.leaveRoom).toHaveBeenCalledWith(5);
  });

  it('updateRooms -> RoomsDispatch.updateRooms', () => {
    RoomPersistence.updateRooms([]);
    expect(RoomsDispatch.updateRooms).toHaveBeenCalledWith([]);
  });

  describe('updateGames', () => {
    it('normalizes game when gameType is missing and room exists', () => {
      const game = { gameType: null, gameTypes: [1] } as any;
      const room = { gametypeMap: { 1: 'Standard' } } as any;
      (RoomsSelectors.getRoom as vi.Mock).mockReturnValue(room);
      RoomPersistence.updateGames(1, [game]);
      expect(NormalizeService.normalizeGameObject).toHaveBeenCalledWith(game, room.gametypeMap);
      expect(RoomsDispatch.updateGames).toHaveBeenCalledWith(1, [game]);
    });

    it('does not normalize when game already has gameType', () => {
      const game = { gameType: 'Standard' } as any;
      RoomPersistence.updateGames(1, [game]);
      expect(NormalizeService.normalizeGameObject).not.toHaveBeenCalled();
    });

    it('does not normalize when room is not found', () => {
      const game = { gameType: null } as any;
      (RoomsSelectors.getRoom as vi.Mock).mockReturnValue(null);
      RoomPersistence.updateGames(1, [game]);
      expect(NormalizeService.normalizeGameObject).not.toHaveBeenCalled();
    });

    it('returns without error when gameList is empty', () => {
      expect(() => RoomPersistence.updateGames(1, [])).not.toThrow();
      expect(RoomsDispatch.updateGames).not.toHaveBeenCalled();
    });

    it('returns without error when gameList is null', () => {
      expect(() => RoomPersistence.updateGames(1, null as any)).not.toThrow();
      expect(RoomsDispatch.updateGames).not.toHaveBeenCalled();
    });
  });

  it('addMessage normalizes message and dispatches', () => {
    const msg = { name: 'alice', message: 'hi' } as any;
    RoomPersistence.addMessage(1, msg);
    expect(NormalizeService.normalizeUserMessage).toHaveBeenCalledWith(msg);
    expect(RoomsDispatch.addMessage).toHaveBeenCalledWith(1, msg);
  });

  it('userJoined -> RoomsDispatch.userJoined', () => {
    const user = { name: 'bob' } as any;
    RoomPersistence.userJoined(1, user);
    expect(RoomsDispatch.userJoined).toHaveBeenCalledWith(1, user);
  });

  it('userLeft -> RoomsDispatch.userLeft', () => {
    RoomPersistence.userLeft(1, 'bob');
    expect(RoomsDispatch.userLeft).toHaveBeenCalledWith(1, 'bob');
  });

  it('removeMessages -> RoomsDispatch.removeMessages', () => {
    RoomPersistence.removeMessages(1, 'bob', 5);
    expect(RoomsDispatch.removeMessages).toHaveBeenCalledWith(1, 'bob', 5);
  });

  it('gameCreated -> RoomsDispatch.gameCreated', () => {
    RoomPersistence.gameCreated(1);
    expect(RoomsDispatch.gameCreated).toHaveBeenCalledWith(1);
  });

  it('joinedGame -> RoomsDispatch.joinedGame', () => {
    RoomPersistence.joinedGame(1, 99);
    expect(RoomsDispatch.joinedGame).toHaveBeenCalledWith(1, 99);
  });
});
