import webClient from '../../WebClient';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  const CmdRemoveFromList = webClient.protobuf.controller.Command_RemoveFromList.create({ list, userName });

  const sc = webClient.protobuf.controller.SessionCommand.create({
    '.Command_RemoveFromList.ext': CmdRemoveFromList
  });

  webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
    // @TODO: filter responseCode, pop snackbar for error
  });
}
