import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  const command = webClient.protobuf.controller.Command_AccountEdit.create({ passwordCheck, realName, email, country });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_AccountEdit.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.accountEditChanged(realName, email, country);
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
