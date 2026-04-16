vi.mock('../../WebClient');

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { WebClient } from '../../WebClient';
import {
  Command_CreateGame_ext,
  Command_CreateGameSchema,
  Command_JoinGame_ext,
  Command_JoinGameSchema,
  Command_LeaveRoom_ext,
  Command_RoomSay_ext,
} from '@app/generated';

import { createGame } from './createGame';
import { joinGame } from './joinGame';
import { leaveRoom } from './leaveRoom';
import { roomSay } from './roomSay';
import { create } from '@bufbuild/protobuf';
import { Mock } from 'vitest';

const { invokeOnSuccess } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendRoomCommand as Mock,
  // sendRoomCommand(roomId, ext, value, options) — options at index 3
  3
);

// ----------------------------------------------------------------
// createGame
// ----------------------------------------------------------------
describe('createGame', () => {

  it('calls sendRoomCommand with Command_CreateGame', () => {
    createGame(5, create(Command_CreateGameSchema, { maxPlayers: 4 }));
    expect(WebClient.instance.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      5, Command_CreateGame_ext, expect.objectContaining({ maxPlayers: 4 }), expect.any(Object)
    );
  });

  it('onSuccess calls response.room.gameCreated with roomId', () => {
    createGame(5, create(Command_CreateGameSchema, {}));
    invokeOnSuccess();
    expect(WebClient.instance.response.room.gameCreated).toHaveBeenCalledWith(5);
  });
});

// ----------------------------------------------------------------
// joinGame
// ----------------------------------------------------------------
describe('joinGame', () => {

  it('calls sendRoomCommand with Command_JoinGame', () => {
    joinGame(7, create(Command_JoinGameSchema, { gameId: 42, password: '' }));
    expect(WebClient.instance.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      7, Command_JoinGame_ext, expect.objectContaining({ gameId: 42, password: '' }), expect.any(Object)
    );
  });

  it('onSuccess calls response.room.joinedGame with roomId and gameId', () => {
    joinGame(7, create(Command_JoinGameSchema, { gameId: 42 }));
    invokeOnSuccess();
    expect(WebClient.instance.response.room.joinedGame).toHaveBeenCalledWith(7, 42);
  });
});

// ----------------------------------------------------------------
// leaveRoom
// ----------------------------------------------------------------
describe('leaveRoom', () => {

  it('calls sendRoomCommand with Command_LeaveRoom', () => {
    leaveRoom(3);
    expect(WebClient.instance.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      3, Command_LeaveRoom_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls response.room.leaveRoom with roomId', () => {
    leaveRoom(3);
    invokeOnSuccess();
    expect(WebClient.instance.response.room.leaveRoom).toHaveBeenCalledWith(3);
  });
});

// ----------------------------------------------------------------
// roomSay
// ----------------------------------------------------------------
describe('roomSay', () => {

  it('calls sendRoomCommand with trimmed message', () => {
    roomSay(2, '  hello  ');
    expect(WebClient.instance.protobuf.sendRoomCommand).toHaveBeenCalledWith(
      2,
      Command_RoomSay_ext,
      expect.objectContaining({ message: 'hello' })
    );
  });

  it('does not call sendRoomCommand when message is blank', () => {
    roomSay(2, '   ');
    expect(WebClient.instance.protobuf.sendRoomCommand).not.toHaveBeenCalled();
  });

  it('does not call sendRoomCommand when message is empty string', () => {
    roomSay(2, '');
    expect(WebClient.instance.protobuf.sendRoomCommand).not.toHaveBeenCalled();
  });
});
