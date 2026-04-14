vi.mock('../../services/BackendService', () => ({
  BackendService: {
    sendRoomCommand: vi.fn(),
  },
}));

vi.mock('../../persistence', () => ({
  RoomPersistence: {
    gameCreated: vi.fn(),
    joinedGame: vi.fn(),
    leaveRoom: vi.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { RoomPersistence } from '../../persistence';
import { Command_CreateGame_ext, Command_JoinGame_ext, Command_LeaveRoom_ext, Command_RoomSay_ext } from 'generated/proto/room_commands_pb';
import { createGame } from './createGame';
import { joinGame } from './joinGame';
import { leaveRoom } from './leaveRoom';
import { roomSay } from './roomSay';

import { Mock } from 'vitest';

const { invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendRoomCommand as Mock,
  // sendRoomCommand(roomId, ext, value, options) — options at index 3
  3
);

beforeEach(() => vi.clearAllMocks());

// ----------------------------------------------------------------
// createGame
// ----------------------------------------------------------------
describe('createGame', () => {

  it('calls sendRoomCommand with Command_CreateGame', () => {
    createGame(5, { maxPlayers: 4 } as any);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(
      5, Command_CreateGame_ext, expect.objectContaining({ maxPlayers: 4 }), expect.any(Object)
    );
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

  it('calls sendRoomCommand with Command_JoinGame', () => {
    joinGame(7, { gameId: 42, password: '' } as any);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(
      7, Command_JoinGame_ext, expect.objectContaining({ gameId: 42, password: '' }), expect.any(Object)
    );
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

  it('calls sendRoomCommand with Command_LeaveRoom', () => {
    leaveRoom(3);
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(3, Command_LeaveRoom_ext, expect.any(Object), expect.any(Object));
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

  it('calls sendRoomCommand with trimmed message', () => {
    roomSay(2, '  hello  ');
    expect(BackendService.sendRoomCommand).toHaveBeenCalledWith(
      2,
      Command_RoomSay_ext,
      expect.objectContaining({ message: 'hello' })
    );
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
