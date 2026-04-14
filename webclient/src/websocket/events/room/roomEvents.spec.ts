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
import { RoomPersistence } from '../../persistence';
import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { removeMessages } from './removeMessages';
import { roomSay } from './roomSay';
import { Event_JoinRoomSchema } from 'generated/proto/event_join_room_pb';
import { Event_LeaveRoomSchema } from 'generated/proto/event_leave_room_pb';
import { Event_ListGamesSchema } from 'generated/proto/event_list_games_pb';
import { Event_RemoveMessagesSchema } from 'generated/proto/event_remove_messages_pb';
import { Event_RoomSaySchema } from 'generated/proto/event_room_say_pb';
import { RoomEventSchema } from 'generated/proto/room_event_pb';

const makeRoomEvent = (roomId: number) => create(RoomEventSchema, { roomId });

beforeEach(() => vi.clearAllMocks());

describe('joinRoom room event', () => {

  it('calls RoomPersistence.userJoined with roomId and userInfo', () => {
    const data = create(Event_JoinRoomSchema, { userInfo: { name: 'alice' } });
    joinRoom(data, makeRoomEvent(3));
    expect(RoomPersistence.userJoined).toHaveBeenCalledWith(3, data.userInfo);
  });
});

describe('leaveRoom room event', () => {

  it('calls RoomPersistence.userLeft with roomId and name', () => {
    leaveRoom(create(Event_LeaveRoomSchema, { name: 'alice' }), makeRoomEvent(4));
    expect(RoomPersistence.userLeft).toHaveBeenCalledWith(4, 'alice');
  });
});

describe('listGames room event', () => {

  it('calls RoomPersistence.updateGames with roomId and gameList', () => {
    const data = create(Event_ListGamesSchema, { gameList: [{ gameId: 1 }] });
    listGames(data, makeRoomEvent(5));
    expect(RoomPersistence.updateGames).toHaveBeenCalledWith(5, data.gameList);
  });
});

describe('removeMessages room event', () => {

  it('calls RoomPersistence.removeMessages with roomId, name, amount', () => {
    removeMessages(create(Event_RemoveMessagesSchema, { name: 'bob', amount: 10 }), makeRoomEvent(6));
    expect(RoomPersistence.removeMessages).toHaveBeenCalledWith(6, 'bob', 10);
  });
});

describe('roomSay room event', () => {
  beforeEach(() => {
    vi.useFakeTimers(); vi.setSystemTime(0);
  });
  afterEach(() => vi.useRealTimers());

  it('calls RoomPersistence.addMessage with roomId and message', () => {
    const data = create(Event_RoomSaySchema, { message: 'hello' });
    roomSay(data, makeRoomEvent(7));
    expect(RoomPersistence.addMessage).toHaveBeenCalledWith(7, { ...data, timeReceived: 0 });
  });
});
