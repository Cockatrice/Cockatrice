import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckList(): void {
  const command = webClient.protobuf.controller.Command_DeckList.create();
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckList.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;
    const response = raw['.Response_DeckList.ext'];

    if (response) {
      switch (responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          SessionPersistence.deckList(response);
          break;
        default:
          console.log('Failed to do the thing');
      }
    }
  });
}
