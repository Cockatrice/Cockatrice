import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function getUserInfo(userName: string): void {
  const command = webClient.protobuf.controller.Command_GetUserInfo.create({ userName });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_GetUserInfo.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { userInfo } = raw['.Response_GetUserInfo.ext'];
        SessionPersistence.getUserInfo(userInfo);
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
