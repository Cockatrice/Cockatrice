import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';
import { Data } from '@app/types';

export function listRooms(): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_ListRooms_ext, create(Data.Command_ListRoomsSchema));
}
