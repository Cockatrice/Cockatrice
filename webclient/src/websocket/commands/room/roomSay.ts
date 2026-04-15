import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function roomSay(roomId: number, message: string): void {
  const trimmed = message.trim();

  if (!trimmed) {
    return;
  }

  webClient.protobuf.sendRoomCommand(roomId, Data.Command_RoomSay_ext, create(Data.Command_RoomSaySchema, { message: trimmed }));
}
