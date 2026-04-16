import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function joinGame(roomId: number, joinGameParams: Data.JoinGameParams): void {
  WebClient.instance.protobuf.sendRoomCommand(roomId, Data.Command_JoinGame_ext, create(Data.Command_JoinGameSchema, joinGameParams), {
    onSuccess: () => {
      WebClient.instance.response.room.joinedGame(roomId, joinGameParams.gameId);
    },
  });
}
