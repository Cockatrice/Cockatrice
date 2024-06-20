import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckNewDir(path: string, dirName: string): void {
  const command = webClient.protobuf.controller.Command_DeckNewDir.create({ path, dirName });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckNewDir.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.deckNewDir(path, dirName);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}
