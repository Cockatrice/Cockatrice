vi.mock('@app/store', () => ({
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
}));

import { RoomPersistence } from './RoomPersistence';
import { RoomsDispatch } from '@app/store';
import { create } from '@bufbuild/protobuf';
import { Data, Enriched } from '@app/types';

describe('RoomPersistence', () => {
  it('clearStore -> RoomsDispatch.clearStore', () => {
    RoomPersistence.clearStore();
    expect(RoomsDispatch.clearStore).toHaveBeenCalled();
  });

  it('joinRoom dispatches raw roomInfo', () => {
    const room = create(Data.ServerInfo_RoomSchema, { roomId: 1 });
    RoomPersistence.joinRoom(room);
    expect(RoomsDispatch.joinRoom).toHaveBeenCalledWith(room);
  });

  it('leaveRoom -> RoomsDispatch.leaveRoom', () => {
    RoomPersistence.leaveRoom(5);
    expect(RoomsDispatch.leaveRoom).toHaveBeenCalledWith(5);
  });

  it('updateRooms dispatches raw rooms', () => {
    const rooms = [create(Data.ServerInfo_RoomSchema, { roomId: 1 })];
    RoomPersistence.updateRooms(rooms);
    expect(RoomsDispatch.updateRooms).toHaveBeenCalledWith(rooms);
  });

  describe('updateGames', () => {
    it('dispatches raw game list', () => {
      const game = create(Data.ServerInfo_GameSchema, { gameTypes: [1] });
      RoomPersistence.updateGames(1, [game]);
      expect(RoomsDispatch.updateGames).toHaveBeenCalledWith(1, [game]);
    });

    it('returns without error when gameList is empty', () => {
      expect(() => RoomPersistence.updateGames(1, [])).not.toThrow();
      expect(RoomsDispatch.updateGames).toHaveBeenCalledWith(1, []);
    });

    it('returns without error when gameList is null', () => {
      expect(() => RoomPersistence.updateGames(1, null as unknown as Data.ServerInfo_Game[])).not.toThrow();
      expect(RoomsDispatch.updateGames).toHaveBeenCalledWith(1, null);
    });
  });

  it('addMessage dispatches without pre-normalizing', () => {
    const msg: Enriched.Message = { ...create(Data.Event_RoomSaySchema), timeReceived: 0, name: 'alice', message: 'hi' };
    RoomPersistence.addMessage(1, msg);
    expect(RoomsDispatch.addMessage).toHaveBeenCalledWith(1, msg);
  });

  it('userJoined -> RoomsDispatch.userJoined', () => {
    const user = create(Data.ServerInfo_UserSchema, { name: 'bob' });
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
