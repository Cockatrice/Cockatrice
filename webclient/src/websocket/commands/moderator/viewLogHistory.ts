import webClient from '../../WebClient';
import {ModeratorPersistence} from '../../persistence';

export function viewLogHistory(filters): void {
  const CmdViewLogHistory = webClient.protobuf.controller.Command_ViewLogHistory.create(filters);

  const sc = webClient.protobuf.controller.ModeratorCommand.create({
    '.Command_ViewLogHistory.ext': CmdViewLogHistory
  });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { logMessage } = raw['.Response_ViewLogHistory.ext'];
        ModeratorPersistence.viewLogs(logMessage)
        return;
      default:
        error = 'Failed to retrieve log history.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
