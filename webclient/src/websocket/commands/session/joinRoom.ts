import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function joinRoom(roomId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_JoinRoom_ext, create(Data.Command_JoinRoomSchema, { roomId }), {
    responseExt: Data.Response_JoinRoom_ext,
    onSuccess: (response) => {
      if (response.roomInfo) {
        WebClient.instance.response.room.joinRoom(response.roomInfo);
      }
    },
  });
}
