import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function leaveRoom(roomId: number): void {
  WebClient.instance.protobuf.sendRoomCommand(roomId, Data.Command_LeaveRoom_ext, create(Data.Command_LeaveRoomSchema), {
    onSuccess: () => {
      WebClient.instance.response.room.leaveRoom(roomId);
    },
  });
}
