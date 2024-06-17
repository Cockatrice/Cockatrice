import webClient from '../../WebClient';

export function listRooms(): void {
  const command = webClient.protobuf.controller.Command_ListRooms.create();
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_ListRooms.ext': command });

  webClient.protobuf.sendSessionCommand(sc);
}
