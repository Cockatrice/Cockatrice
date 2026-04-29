vi.mock('../../WebClient');

import { create } from '@bufbuild/protobuf';
import {
  Event_JoinRoomSchema,
  Event_LeaveRoomSchema,
  Event_ListGamesSchema,
  Event_RemoveMessagesSchema,
  Event_RoomSaySchema,
  RoomEventSchema,
} from '@app/generated';
import { WebClient } from '../../WebClient';
import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { removeMessages } from './removeMessages';
import { roomSay } from './roomSay';

const makeRoomEvent = (roomId: number) => create(RoomEventSchema, { roomId });

describe('joinRoom room event', () => {

  it('calls response.room.userJoined with roomId and userInfo', () => {
    const data = create(Event_JoinRoomSchema, { userInfo: { name: 'alice' } });
    joinRoom(data, makeRoomEvent(3));
    expect(WebClient.instance.response.room.userJoined).toHaveBeenCalledWith(3, data.userInfo);
  });
});

describe('leaveRoom room event', () => {

  it('calls response.room.userLeft with roomId and name', () => {
    leaveRoom(create(Event_LeaveRoomSchema, { name: 'alice' }), makeRoomEvent(4));
    expect(WebClient.instance.response.room.userLeft).toHaveBeenCalledWith(4, 'alice');
  });
});

describe('listGames room event', () => {

  it('calls response.room.updateGames with roomId and gameList', () => {
    const data = create(Event_ListGamesSchema, { gameList: [{ gameId: 1 }] });
    listGames(data, makeRoomEvent(5));
    expect(WebClient.instance.response.room.updateGames).toHaveBeenCalledWith(5, data.gameList);
  });
});

describe('removeMessages room event', () => {

  it('calls response.room.removeMessages with roomId, name, amount', () => {
    removeMessages(create(Event_RemoveMessagesSchema, { name: 'bob', amount: 10 }), makeRoomEvent(6));
    expect(WebClient.instance.response.room.removeMessages).toHaveBeenCalledWith(6, 'bob', 10);
  });
});

describe('roomSay room event', () => {
  beforeEach(() => {
    vi.useFakeTimers(); vi.setSystemTime(0);
  });
  afterEach(() => vi.useRealTimers());

  it('calls response.room.addMessage with roomId and message', () => {
    const data = create(Event_RoomSaySchema, { message: 'hello' });
    roomSay(data, makeRoomEvent(7));
    expect(WebClient.instance.response.room.addMessage).toHaveBeenCalledWith(7, { ...data, timeReceived: 0 });
  });
});
