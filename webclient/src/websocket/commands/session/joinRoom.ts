import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { RoomPersistence } from '../../persistence';
import { Data } from '@app/types';

export function joinRoom(roomId: number): void {
  webClient.protobuf.sendSessionCommand(Data.Command_JoinRoom_ext, create(Data.Command_JoinRoomSchema, { roomId }), {
    responseExt: Data.Response_JoinRoom_ext,
    onSuccess: (response) => {
      if (response.roomInfo) {
        RoomPersistence.joinRoom(response.roomInfo);
      }
    },
  });
}
