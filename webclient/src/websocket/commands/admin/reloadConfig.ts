import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function reloadConfig(): void {
  const command = webClient.protobuf.controller.Command_ReloadConfig.create();
  const sc = webClient.protobuf.controller.AdminCommand.create({ '.Command_ReloadConfig.ext': command });

  webClient.protobuf.sendAdminCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        AdminPersistence.reloadConfig();
        return;
      default:
        error = 'Failed to reload config.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
