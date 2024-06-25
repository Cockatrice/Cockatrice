import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckDel(deckId: number): void {
  const command = webClient.protobuf.controller.Command_DeckDel.create({ deckId });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckDel.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.deckDelete(deckId);
        break;
      default:
        console.log('Failed to do the thing');
    }
  });
}
