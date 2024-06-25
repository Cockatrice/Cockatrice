import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckDownload(deckId: number): void {
  const command = webClient.protobuf.controller.Command_DeckDownload.create({ deckId });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckDownload.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.deckDownload(deckId);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}
