vi.mock('../../WebClient', () => ({
  __esModule: true,
  default: { protobuf: { sendRoomCommand: vi.fn() } },
}));

vi.mock('../../persistence', () => ({
  RoomPersistence: {
    gameCreated: vi.fn(),
    joinedGame: vi.fn(),
    leaveRoom: vi.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import webClient from '../../WebClient';
import { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';

import { createGame } from './createGame';
import { joinGame } from './joinGame';
import { leaveRoom } from './leaveRoom';
import { roomSay } from './roomSay';
import { create } from '@bufbuild/protobuf';
import { Mock } from 'vitest';

const { invokeOnSuccess } = makeCallbackHelpers(
  webClient.protobuf.sendRoomCommand as Mock,
  // sendRoomCommand(roomId, ext, value, options) — options at index 3
  3
);

// ----------------------------------------------------------------
// createGame
// ----------------------------------------------------------------
describe('createGame', () => {

  it('calls sendRoomCommand with Command_CreateGame', () => {
    createGame(5, create(Data.Command_CreateGameSchema, { maxPlayers: 4 }));
    expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      5, Data.Command_CreateGame_ext, expect.objectContaining({ maxPlayers: 4 }), expect.any(Object)
    );
  });

  it('onSuccess calls RoomPersistence.gameCreated with roomId', () => {
    createGame(5, create(Data.Command_CreateGameSchema, {}));
    invokeOnSuccess();
    expect(RoomPersistence.gameCreated).toHaveBeenCalledWith(5);
  });
});

// ----------------------------------------------------------------
// joinGame
// ----------------------------------------------------------------
describe('joinGame', () => {

  it('calls sendRoomCommand with Command_JoinGame', () => {
    joinGame(7, create(Data.Command_JoinGameSchema, { gameId: 42, password: '' }));
    expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      7, Data.Command_JoinGame_ext, expect.objectContaining({ gameId: 42, password: '' }), expect.any(Object)
    );
  });

  it('onSuccess calls RoomPersistence.joinedGame with roomId and gameId', () => {
    joinGame(7, create(Data.Command_JoinGameSchema, { gameId: 42 }));
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
    expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(3, Data.Command_LeaveRoom_ext, expect.any(Object), expect.any(Object));
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
    expect(webClient.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      2,
      Data.Command_RoomSay_ext,
      expect.objectContaining({ message: 'hello' })
    );
  });

  it('does not call sendRoomCommand when message is blank', () => {
    roomSay(2, '   ');
    expect(webClient.protobuf.sendRoomCommand).not.toHaveBeenCalled();
  });

  it('does not call sendRoomCommand when message is empty string', () => {
    roomSay(2, '');
    expect(webClient.protobuf.sendRoomCommand).not.toHaveBeenCalled();
  });
});
