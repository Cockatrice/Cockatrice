jest.mock('../../services/BackendService', () => ({
  BackendService: {
    sendRoomCommand: jest.fn(),
  },
}));

jest.mock('../../persistence', () => ({
  RoomPersistence: {
    gameCreated: jest.fn(),
    joinedGame: jest.fn(),
    leaveRoom: jest.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';

const { getLastSendOpts, invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendRoomCommand as jest.Mock,
  // sendRoomCommand(roomId, commandName, params, options) — options at index 3
  3
);

beforeEach(() => jest.clearAllMocks());

// ----------------------------------------------------------------
// createGame
// ----------------------------------------------------------------
describe('createGame', () => {
  const { createGame } = jest.requireActual('./createGame');

  it('calls sendRoomCommand with Command_CreateGame', () => {
    createGame(5, { maxPlayers: 4 } as any);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(5, 'Command_CreateGame', { maxPlayers: 4 }, expect.any(Object));
  });

  it('onSuccess calls RoomPersistence.gameCreated with roomId', () => {
    createGame(5, {} as any);
    invokeOnSuccess();
    expect(RoomPersistence.gameCreated).toHaveBeenCalledWith(5);
  });
});

// ----------------------------------------------------------------
// joinGame
// ----------------------------------------------------------------
describe('joinGame', () => {
  const { joinGame } = jest.requireActual('./joinGame');

  it('calls sendRoomCommand with Command_JoinGame', () => {
    joinGame(7, { gameId: 42, password: '' } as any);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(7, 'Command_JoinGame', { gameId: 42, password: '' }, expect.any(Object));
  });

  it('onSuccess calls RoomPersistence.joinedGame with roomId and gameId', () => {
    joinGame(7, { gameId: 42 } as any);
    invokeOnSuccess();
    expect(RoomPersistence.joinedGame).toHaveBeenCalledWith(7, 42);
  });
});

// ----------------------------------------------------------------
// leaveRoom
// ----------------------------------------------------------------
describe('leaveRoom', () => {
  const { leaveRoom } = jest.requireActual('./leaveRoom');

  it('calls sendRoomCommand with Command_LeaveRoom', () => {
    leaveRoom(3);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(3, 'Command_LeaveRoom', {}, expect.any(Object));
  });

  it('onSuccess calls RoomPersistence.leaveRoom with roomId', () => {
    leaveRoom(3);
    invokeOnSuccess();
    expect(RoomPersistence.leaveRoom).toHaveBeenCalledWith(3);
  });
});

// ----------------------------------------------------------------
// roomSay
// ----------------------------------------------------------------
describe('roomSay', () => {
  const { roomSay } = jest.requireActual('./roomSay');

  it('calls sendRoomCommand with trimmed message', () => {
    roomSay(2, '  hello  ');
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(2, 'Command_RoomSay', { message: 'hello' }, expect.any(Object));
  });

  it('does not call sendRoomCommand when message is blank', () => {
    roomSay(2, '   ');
    expect(BackendService.sendRoomCommand).not.toHaveBeenCalled();
  });

  it('does not call sendRoomCommand when message is empty string', () => {
    roomSay(2, '');
    expect(BackendService.sendRoomCommand).not.toHaveBeenCalled();
  });
});
