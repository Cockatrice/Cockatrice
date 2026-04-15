import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Data } from '@app/types';

export function listRooms(): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ListRooms_ext, create(Data.Command_ListRoomsSchema));
}
