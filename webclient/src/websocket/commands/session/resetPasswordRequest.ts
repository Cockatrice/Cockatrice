import { ForgotPasswordParams } from "store";
import { StatusEnum, WebSocketConnectOptions } from "types";

import webClient from "../../WebClient";
import { SessionPersistence } from "../../persistence";

import { disconnect, updateStatus } from "./";

export function resetPasswordRequest(options: WebSocketConnectOptions): void {
  const { userName } = options as unknown as ForgotPasswordParams;

  const forgotPasswordConfig = {
    ...webClient.clientConfig,
    userName,
  };

  const CmdForgotPasswordRequest = webClient.protobuf.controller.Command_ForgotPasswordRequest.create(forgotPasswordConfig);

  const sc = webClient.protobuf.controller.SessionCommand.create({
    '.Command_ForgotPasswordRequest.ext': CmdForgotPasswordRequest
  });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
      const resp = raw['.Response_ForgotPasswordRequest.ext'];

      if (resp.challengeEmail) {
        updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordChallenge();
      } else {
        updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPassword();
      }
    } else {
      updateStatus(StatusEnum.DISCONNECTED, null);
      SessionPersistence.resetPasswordFailed();
    }

    disconnect();
  });
}
