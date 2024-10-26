import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';

export function getBanHistory(userName: string): void {
  const command = webClient.protobuf.controller.Command_GetBanHistory.create({ userName });
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_GetBanHistory.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { banList } = raw['.Response_BanHistory.ext'];
        ModeratorPersistence.banHistory(userName, banList);
        return;
      default:
        error = 'Failed to get ban history.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
