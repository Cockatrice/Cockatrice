import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ListRooms_ext, Command_ListRoomsSchema } from '@app/generated';

export function listRooms(): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_ListRooms_ext, create(Command_ListRoomsSchema));
}
