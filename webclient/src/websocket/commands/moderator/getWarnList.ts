import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  const command = webClient.protobuf.controller.Command_GetWarnList.create({ modName, userName, userClientid });
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_GetWarnList.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { warning } = raw['.Response_WarnList.ext'];
        ModeratorPersistence.warnListOptions(warning);
        return;
      default:
        error = 'Failed to get warn list.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
