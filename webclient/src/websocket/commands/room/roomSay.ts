import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_RoomSay_ext, Command_RoomSaySchema } from 'generated/proto/room_commands_pb';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  BackendService.sendRoomCommand(roomId, Command_RoomSay_ext, create(Command_RoomSaySchema, { message: trimmed }));
}
