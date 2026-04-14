import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_LeaveRoom_ext, Command_LeaveRoomSchema } from 'generated/proto/room_commands_pb';
import { RoomPersistence } from '../../persistence';

export function leaveRoom(roomId: number): void {
  webClient.protobuf.sendRoomCommand(roomId, Command_LeaveRoom_ext, create(Command_LeaveRoomSchema), {
    onSuccess: () => {
      RoomPersistence.leaveRoom(roomId);
    },
  });
}
