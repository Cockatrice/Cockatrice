import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function listUsers(): void {
  const command = webClient.protobuf.controller.Command_ListUsers.create();
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_ListUsers.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    const { responseCode } = raw;
    const response = raw['.Response_ListUsers.ext'];

    if (response) {
      switch (responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          SessionPersistence.updateUsers(response.userList);
          break;
        default:
          console.log(`Failed to fetch Server Rooms [${responseCode}] : `, raw);
      }
    }

  });
}
