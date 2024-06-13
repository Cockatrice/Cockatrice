import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function updateServerMessage(): void {
  const command = webClient.protobuf.controller.Command_GetBanHistory.create();

  const sc = webClient.protobuf.controller.ModeratorCommand.create({
    '.Command_UpdateServerMessage.ext': command
  });

  webClient.protobuf.sendAdminCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        AdminPersistence.updateServerMessage();
        return;
      default:
        error = 'Failed to update server message.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
