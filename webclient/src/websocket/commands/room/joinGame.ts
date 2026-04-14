import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_JoinGame_ext, Command_JoinGameSchema } from 'generated/proto/room_commands_pb';
import { RoomPersistence } from '../../persistence';
import { JoinGameParams } from 'types';

export function joinGame(roomId: number, joinGameParams: JoinGameParams): void {
  webClient.protobuf.sendRoomCommand(roomId, Command_JoinGame_ext, create(Command_JoinGameSchema, joinGameParams), {
    onSuccess: () => {
      RoomPersistence.joinedGame(roomId, joinGameParams.gameId);
    },
  });
}
