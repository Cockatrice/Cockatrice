vi.mock('../../persistence', () => ({
  RoomPersistence: {
    userJoined: vi.fn(),
    userLeft: vi.fn(),
    updateGames: vi.fn(),
    removeMessages: vi.fn(),
    addMessage: vi.fn(),
  },
}));

import { RoomPersistence } from '../../persistence';
import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { removeMessages } from './removeMessages';
import { roomSay } from './roomSay';

const makeRoomEvent = (roomId: number) => ({ roomEvent: { roomId } });

beforeEach(() => vi.clearAllMocks());

describe('joinRoom room event', () => {

  it('calls RoomPersistence.userJoined with roomId and userInfo', () => {
    const userInfo = { name: 'alice' } as any;
    joinRoom({ userInfo }, makeRoomEvent(3));
    expect(RoomPersistence.userJoined).toHaveBeenCalledWith(3, userInfo);
  });
});

describe('leaveRoom room event', () => {

  it('calls RoomPersistence.userLeft with roomId and name', () => {
    leaveRoom({ name: 'alice' }, makeRoomEvent(4));
    expect(RoomPersistence.userLeft).toHaveBeenCalledWith(4, 'alice');
  });
});

describe('listGames room event', () => {

  it('calls RoomPersistence.updateGames with roomId and gameList', () => {
    const gameList = [{ gameId: 1 }] as any;
    listGames({ gameList }, makeRoomEvent(5));
    expect(RoomPersistence.updateGames).toHaveBeenCalledWith(5, gameList);
  });
});

describe('removeMessages room event', () => {

  it('calls RoomPersistence.removeMessages with roomId, name, amount', () => {
    removeMessages({ name: 'bob', amount: 10 }, makeRoomEvent(6));
    expect(RoomPersistence.removeMessages).toHaveBeenCalledWith(6, 'bob', 10);
  });
});

describe('roomSay room event', () => {

  it('calls RoomPersistence.addMessage with roomId and message', () => {
    const msg = { text: 'hello' } as any;
    roomSay(msg, makeRoomEvent(7));
    expect(RoomPersistence.addMessage).toHaveBeenCalledWith(7, msg);
  });
});
