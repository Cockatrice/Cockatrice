import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  const command = webClient.protobuf.controller.Command_BanFromServer.create({
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  });
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_BanFromServer.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        ModeratorPersistence.banFromServer(userName);
        return;
      default:
        error = 'Failed to ban user.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
