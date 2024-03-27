import webClient from '../../WebClient';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  const CmdRoomSay = webClient.protobuf.controller.Command_RoomSay.create({
    'message': trimmed
  });

  const rc = webClient.protobuf.controller.RoomCommand.create({
    '.Command_RoomSay.ext': CmdRoomSay
  });

  webClient.protobuf.sendRoomCommand(roomId, rc);
}
