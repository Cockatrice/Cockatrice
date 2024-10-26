import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  const command = webClient.protobuf.controller.Command_RemoveFromList.create({ list, userName });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_RemoveFromList.ext': command });

  webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.removeFromList(list, userName);
        break;
      default:
        console.error('Failed to remove from list', responseCode);
    }
  });
}
