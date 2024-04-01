import webClient from '../../WebClient';

export function listRooms(): void {
  const CmdListRooms = webClient.protobuf.controller.Command_ListRooms.create();

  const sc = webClient.protobuf.controller.SessionCommand.create({
    '.Command_ListRooms.ext': CmdListRooms
  });

  webClient.protobuf.sendSessionCommand(sc);
}
