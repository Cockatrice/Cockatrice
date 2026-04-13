import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_LeaveRoom_ext, Command_LeaveRoomSchema } from 'generated/proto/room_commands_pb';
import { RoomPersistence } from '../../persistence';

export function leaveRoom(roomId: number): void {
  BackendService.sendRoomCommand(roomId, Command_LeaveRoom_ext, create(Command_LeaveRoomSchema), {
    onSuccess: () => {
      RoomPersistence.leaveRoom(roomId);
    },
  });
}
