import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_JoinRoom_ext, Command_JoinRoomSchema, Response_JoinRoom_ext } from '@app/generated';

export function joinRoom(roomId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_JoinRoom_ext, create(Command_JoinRoomSchema, { roomId }), {
    responseExt: Response_JoinRoom_ext,
    onSuccess: (response) => {
      if (response.roomInfo) {
        WebClient.instance.response.room.joinRoom(response.roomInfo);
      }
    },
  });
}
