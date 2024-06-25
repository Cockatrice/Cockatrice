import { RoomPersistence } from '../../persistence';
import webClient from '../../WebClient';
import { GameConfig } from 'types';

export function createGame(roomId: number, gameConfig: GameConfig): void {
  const command = webClient.protobuf.controller.Command_CreateGame.create(gameConfig);
  const rc = webClient.protobuf.controller.RoomCommand.create({ '.Command_CreateGame.ext': command });

  webClient.protobuf.sendRoomCommand(roomId, rc, (raw) => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        RoomPersistence.gameCreated(roomId);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}

