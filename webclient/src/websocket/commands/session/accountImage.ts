import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';
import { common } from 'protobufjs';
import IBytesValue = common.IBytesValue;

export function accountImage(image: IBytesValue): void {
  const command = webClient.protobuf.controller.Command_AccountImage.create({ image });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_AccountImage.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.accountImageChanged(image);
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespFunctionNotAllowed:
        console.log('Not allowed');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword:
        console.log('Wrong password');
        break;
      default:
        console.log('Failed to update information');
    }
  });
}
