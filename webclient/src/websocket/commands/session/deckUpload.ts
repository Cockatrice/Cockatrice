import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function deckUpload(path: string, deckId: number, deckList: string): void {
  const command = webClient.protobuf.controller.Command_DeckUpload.create({ path, deckId, deckList });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_DeckUpload.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;
    const response = raw['.Response_DeckUpload.ext'];

    if (response) {
      switch (responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          SessionPersistence.deckUpload(response);
          break;
        default:
          console.log('Failed to do the thing');
      }
    }

  });
}
