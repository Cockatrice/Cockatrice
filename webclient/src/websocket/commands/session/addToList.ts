import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  const command = webClient.protobuf.controller.Command_AddToList.create({ list, userName });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_AddToList.ext': command });

  webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        SessionPersistence.addToList(list, userName);
        break;
      default:
        console.error('Failed to add to list', responseCode);
    }
  });
}
