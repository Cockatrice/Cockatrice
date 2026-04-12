jest.mock('../../persistence', () => ({
  RoomPersistence: {
    userJoined: jest.fn(),
    userLeft: jest.fn(),
    updateGames: jest.fn(),
    removeMessages: jest.fn(),
    addMessage: jest.fn(),
  },
}));

import { RoomPersistence } from '../../persistence';

const makeRoomEvent = (roomId: number) => ({ roomEvent: { roomId } });

beforeEach(() => jest.clearAllMocks());

describe('joinRoom room event', () => {
  const { joinRoom } = jest.requireActual('./joinRoom');

  it('calls RoomPersistence.userJoined with roomId and userInfo', () => {
    const userInfo = { name: 'alice' } as any;
    joinRoom({ userInfo }, makeRoomEvent(3));
    expect(RoomPersistence.userJoined).toHaveBeenCalledWith(3, userInfo);
  });
});

describe('leaveRoom room event', () => {
  const { leaveRoom } = jest.requireActual('./leaveRoom');

  it('calls RoomPersistence.userLeft with roomId and name', () => {
    leaveRoom({ name: 'alice' }, makeRoomEvent(4));
    expect(RoomPersistence.userLeft).toHaveBeenCalledWith(4, 'alice');
  });
});

describe('listGames room event', () => {
  const { listGames } = jest.requireActual('./listGames');

  it('calls RoomPersistence.updateGames with roomId and gameList', () => {
    const gameList = [{ gameId: 1 }] as any;
    listGames({ gameList }, makeRoomEvent(5));
    expect(RoomPersistence.updateGames).toHaveBeenCalledWith(5, gameList);
  });
});

describe('removeMessages room event', () => {
  const { removeMessages } = jest.requireActual('./removeMessages');

  it('calls RoomPersistence.removeMessages with roomId, name, amount', () => {
    removeMessages({ name: 'bob', amount: 10 }, makeRoomEvent(6));
    expect(RoomPersistence.removeMessages).toHaveBeenCalledWith(6, 'bob', 10);
  });
});

describe('roomSay room event', () => {
  const { roomSay } = jest.requireActual('./roomSay');

  it('calls RoomPersistence.addMessage with roomId and message', () => {
    const msg = { text: 'hello' } as any;
    roomSay(msg, makeRoomEvent(7));
    expect(RoomPersistence.addMessage).toHaveBeenCalledWith(7, msg);
  });
});
