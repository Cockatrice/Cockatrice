vi.mock('../../persistence', () => ({
  RoomPersistence: {
    userJoined: vi.fn(),
    userLeft: vi.fn(),
    updateGames: vi.fn(),
    removeMessages: vi.fn(),
    addMessage: vi.fn(),
  },
}));

import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';
import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { removeMessages } from './removeMessages';
import { roomSay } from './roomSay';

const makeRoomEvent = (roomId: number) => create(Data.RoomEventSchema, { roomId });

describe('joinRoom room event', () => {

  it('calls RoomPersistence.userJoined with roomId and userInfo', () => {
    const data = create(Data.Event_JoinRoomSchema, { userInfo: { name: 'alice' } });
    joinRoom(data, makeRoomEvent(3));
    expect(RoomPersistence.userJoined).toHaveBeenCalledWith(3, data.userInfo);
  });
});

describe('leaveRoom room event', () => {

  it('calls RoomPersistence.userLeft with roomId and name', () => {
    leaveRoom(create(Data.Event_LeaveRoomSchema, { name: 'alice' }), makeRoomEvent(4));
    expect(RoomPersistence.userLeft).toHaveBeenCalledWith(4, 'alice');
  });
});

describe('listGames room event', () => {

  it('calls RoomPersistence.updateGames with roomId and gameList', () => {
    const data = create(Data.Event_ListGamesSchema, { gameList: [{ gameId: 1 }] });
    listGames(data, makeRoomEvent(5));
    expect(RoomPersistence.updateGames).toHaveBeenCalledWith(5, data.gameList);
  });
});

describe('removeMessages room event', () => {

  it('calls RoomPersistence.removeMessages with roomId, name, amount', () => {
    removeMessages(create(Data.Event_RemoveMessagesSchema, { name: 'bob', amount: 10 }), makeRoomEvent(6));
    expect(RoomPersistence.removeMessages).toHaveBeenCalledWith(6, 'bob', 10);
  });
});

describe('roomSay room event', () => {
  beforeEach(() => {
    vi.useFakeTimers(); vi.setSystemTime(0);
  });
  afterEach(() => vi.useRealTimers());

  it('calls RoomPersistence.addMessage with roomId and message', () => {
    const data = create(Data.Event_RoomSaySchema, { message: 'hello' });
    roomSay(data, makeRoomEvent(7));
    expect(RoomPersistence.addMessage).toHaveBeenCalledWith(7, { ...data, timeReceived: 0 });
  });
});
