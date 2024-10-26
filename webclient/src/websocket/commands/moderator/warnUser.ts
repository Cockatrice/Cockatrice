import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';

export function warnUser(userName: string, reason: string, clientid?: string, removeMessage?: boolean): void {
  const command = webClient.protobuf.controller.Command_WarnUser.create({ userName, reason, clientid, removeMessage });
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_WarnUser.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        ModeratorPersistence.warnUser(userName);
        return;
      default:
        error = 'Failed to warn user.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
