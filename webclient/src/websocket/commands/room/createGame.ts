import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_CreateGame_ext, Command_CreateGameSchema } from 'generated/proto/room_commands_pb';
import { RoomPersistence } from '../../persistence';
import { GameConfig } from 'types';

export function createGame(roomId: number, gameConfig: GameConfig): void {
  BackendService.sendRoomCommand(roomId, Command_CreateGame_ext, create(Command_CreateGameSchema, gameConfig), {
    onSuccess: () => {
      RoomPersistence.gameCreated(roomId);
    },
  });
}
