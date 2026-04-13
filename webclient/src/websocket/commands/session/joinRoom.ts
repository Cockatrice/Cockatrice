import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_JoinRoom_ext, Command_JoinRoomSchema } from 'generated/proto/session_commands_pb';
import { RoomPersistence } from '../../persistence';
import { Response_JoinRoom_ext } from 'generated/proto/response_join_room_pb';

export function joinRoom(roomId: number): void {
  BackendService.sendSessionCommand(Command_JoinRoom_ext, create(Command_JoinRoomSchema, { roomId }), {
    responseExt: Response_JoinRoom_ext,
    onSuccess: (response) => {
      RoomPersistence.joinRoom(response.roomInfo);
    },
  });
}
