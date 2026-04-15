import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { RoomPersistence } from '../../persistence';
import { Data } from '@app/types';

export function joinGame(roomId: number, joinGameParams: Data.JoinGameParams): void {
  webClient.protobuf.sendRoomCommand(roomId, Data.Command_JoinGame_ext, create(Data.Command_JoinGameSchema, joinGameParams), {
    onSuccess: () => {
      RoomPersistence.joinedGame(roomId, joinGameParams.gameId);
    },
  });
}
