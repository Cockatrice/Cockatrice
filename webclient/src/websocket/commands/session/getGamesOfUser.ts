import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function getGamesOfUser(userName: string): void {
  const command = webClient.protobuf.controller.Command_GetGamesOfUser.create({ userName });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_GetGamesOfUser.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;
    const response = raw['.Response_GetGamesOfUser.ext'];

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.getGamesOfUser(userName, response);
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespFunctionNotAllowed:
        console.log('Not allowed');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword:
        console.log('Wrong password');
        break;
      default:
        console.log('Failed to update information');
    }
  });
}
