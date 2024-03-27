import webClient from "../../WebClient";

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  const CmdAddToList = webClient.protobuf.controller.Command_AddToList.create({ list, userName });

  const sc = webClient.protobuf.controller.SessionCommand.create({
    '.Command_AddToList.ext': CmdAddToList
  });

  webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
    // @TODO: filter responseCode, pop snackbar for error
  });
}