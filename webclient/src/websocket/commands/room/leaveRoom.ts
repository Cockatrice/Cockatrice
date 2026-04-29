import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_LeaveRoom_ext, Command_LeaveRoomSchema } from '@app/generated';

export function leaveRoom(roomId: number): void {
  WebClient.instance.protobuf.sendRoomCommand(roomId, Command_LeaveRoom_ext, create(Command_LeaveRoomSchema), {
    onSuccess: () => {
      WebClient.instance.response.room.leaveRoom(roomId);
    },
  });
}
