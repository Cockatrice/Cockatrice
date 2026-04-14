import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_CreateGame_ext, Command_CreateGameSchema } from 'generated/proto/room_commands_pb';
import { RoomPersistence } from '../../persistence';
import { GameConfig } from 'types';

export function createGame(roomId: number, gameConfig: GameConfig): void {
  webClient.protobuf.sendRoomCommand(roomId, Command_CreateGame_ext, create(Command_CreateGameSchema, gameConfig), {
    onSuccess: () => {
      RoomPersistence.gameCreated(roomId);
    },
  });
}
