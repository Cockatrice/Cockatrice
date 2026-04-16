import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_JoinGame_ext, Command_JoinGameSchema } from '@app/generated';
import type { JoinGameParams } from '@app/generated';

export function joinGame(roomId: number, joinGameParams: JoinGameParams): void {
  WebClient.instance.protobuf.sendRoomCommand(roomId, Command_JoinGame_ext, create(Command_JoinGameSchema, joinGameParams), {
    onSuccess: () => {
      WebClient.instance.response.room.joinedGame(roomId, joinGameParams.gameId);
    },
  });
}
