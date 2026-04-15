import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { RoomPersistence } from '../../persistence';
import { Data } from '@app/types';

export function leaveRoom(roomId: number): void {
  webClient.protobuf.sendRoomCommand(roomId, Data.Command_LeaveRoom_ext, create(Data.Command_LeaveRoomSchema), {
    onSuccess: () => {
      RoomPersistence.leaveRoom(roomId);
    },
  });
}
