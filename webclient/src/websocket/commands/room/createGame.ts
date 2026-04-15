import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { RoomPersistence } from '../../persistence';
import { Data } from '@app/types';

export function createGame(roomId: number, gameConfig: Data.CreateGameParams): void {
  webClient.protobuf.sendRoomCommand(roomId, Data.Command_CreateGame_ext, create(Data.Command_CreateGameSchema, gameConfig), {
    onSuccess: () => {
      RoomPersistence.gameCreated(roomId);
    },
  });
}
