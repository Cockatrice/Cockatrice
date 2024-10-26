import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  const command = webClient.protobuf.controller.Command_AccountPassword.create({ oldPassword, newPassword, hashedNewPassword });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_AccountPassword.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.accountPasswordChange();
        break;
      default:
        console.log('Failed to change password');
    }
  });
}
