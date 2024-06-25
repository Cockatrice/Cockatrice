import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckDelDir(path: string): void {
  const command = webClient.protobuf.controller.Command_DeckDelDir.create({ path });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckDelDir.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.deckDeleteDir(path);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}
