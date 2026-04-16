import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function createGame(roomId: number, gameConfig: Data.CreateGameParams): void {
  WebClient.instance.protobuf.sendRoomCommand(roomId, Data.Command_CreateGame_ext, create(Data.Command_CreateGameSchema, gameConfig), {
    onSuccess: () => {
      WebClient.instance.response.room.gameCreated(roomId);
    },
  });
}
