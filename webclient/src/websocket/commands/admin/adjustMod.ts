import webClient from '../../WebClient';
import { AdminPersistence } from '../../persistence';

export function adjustMod(userName: string, shouldBeMod?: boolean, shouldBeJudge?: boolean): void {
  const command = webClient.protobuf.controller.Command_AdjustMod.create({ userName, shouldBeMod, shouldBeJudge });
  const sc = webClient.protobuf.controller.AdminCommand.create({ '.Command_AdjustMod.ext': command });

  webClient.protobuf.sendAdminCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        AdminPersistence.adjustMod(userName, shouldBeMod, shouldBeJudge);
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
