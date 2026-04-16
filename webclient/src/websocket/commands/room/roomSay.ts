import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  WebClient.instance.protobuf.sendRoomCommand(roomId, Data.Command_RoomSay_ext, create(Data.Command_RoomSaySchema, { message: trimmed }));
}
