import { RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import { GameConfig, JoinGameParams } from 'types';

export function joinGame(roomId: number, joinGameParams: JoinGameParams): void {
  const command = webClient.protobuf.controller.Command_JoinGame.create(joinGameParams);
  const rc = webClient.protobuf.controller.RoomCommand.create({ '.Command_JoinGame.ext': command });

  webClient.protobuf.sendRoomCommand(roomId, rc, (raw) => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        RoomPersistence.joinedGame(roomId, joinGameParams.gameId);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}

