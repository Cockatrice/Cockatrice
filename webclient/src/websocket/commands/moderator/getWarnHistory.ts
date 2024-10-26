import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';

export function getWarnHistory(userName: string): void {
  const command = webClient.protobuf.controller.Command_GetWarnHistory.create({ userName });
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_GetWarnHistory.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { warnList } = raw['.Response_WarnHistory.ext'];
        ModeratorPersistence.warnHistory(userName, warnList);
        return;
      default:
        error = 'Failed to get warn history.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
