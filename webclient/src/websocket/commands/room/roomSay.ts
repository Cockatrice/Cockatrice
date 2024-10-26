import webClient from '../../WebClient';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  const command = webClient.protobuf.controller.Command_RoomSay.create({ 'message': trimmed });
  const rc = webClient.protobuf.controller.RoomCommand.create({ '.Command_RoomSay.ext': command });

  webClient.protobuf.sendRoomCommand(roomId, rc);
}
