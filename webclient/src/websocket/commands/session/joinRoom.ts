import webClient from '../../WebClient';
import { RoomPersistence } from '../../persistence';

export function joinRoom(roomId: number): void {
  const command = webClient.protobuf.controller.Command_JoinRoom.create({ roomId });
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_JoinRoom.ext': command });

  webClient.protobuf.sendSessionCommand(sc, (raw) => {
    const { responseCode } = raw;

    let error: string;

    switch (responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespOk:
        const { roomInfo } = raw['.Response_JoinRoom.ext'];

        RoomPersistence.joinRoom(roomInfo);
        return;
      case webClient.protobuf.controller.Response.ResponseCode.RespNameNotFound:
        error = 'Failed to join the room: it doesn\'t exist on the server.';
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespContextError:
        error = 'The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.';
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespUserLevelTooLow:
        error = 'You do not have the required permission to join this room.';
        break;
      default:
        error = 'Failed to join the room due to an unknown error.';
        break;
    }

    if (error) {
      console.error(responseCode, error);
    }
  });
}
