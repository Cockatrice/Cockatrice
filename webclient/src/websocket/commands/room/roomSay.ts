import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Command_RoomSay_ext, Command_RoomSaySchema } from '@app/generated';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  WebClient.instance.protobuf.sendRoomCommand(roomId, Command_RoomSay_ext, create(Command_RoomSaySchema, { message: trimmed }));
}
