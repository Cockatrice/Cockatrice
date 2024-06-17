import webClient from '../../WebClient';
import { ModeratorPersistence } from '../../persistence';
import { LogFilters } from 'types';

export function viewLogHistory(filters: LogFilters): void {
  const command = webClient.protobuf.controller.Command_ViewLogHistory.create(filters);
  const sc = webClient.protobuf.controller.ModeratorCommand.create({ '.Command_ViewLogHistory.ext': command });

  webClient.protobuf.sendModeratorCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

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
