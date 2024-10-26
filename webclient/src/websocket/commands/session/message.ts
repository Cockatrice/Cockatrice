import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function message(userName: string, message: string): void {
  const command = webClient.protobuf.controller.Command_Message.create({ userName, message });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_Message.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.directMessageSent(userName, message);
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespNameNotFound:
        console.log('Name not found');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespInIgnoreList:
        console.log('On ignore list');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespChatFlood:
        console.log('Flooding chat');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword:
        console.log('Wrong password');
        break;
      default:
        console.log('Failed to send direct message');
    }
  });
}
