import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ListRooms_ext, Command_ListRoomsSchema } from 'generated/proto/session_commands_pb';

export function listRooms(): void {
  webClient.protobuf.sendSessionCommand(Command_ListRooms_ext, create(Command_ListRoomsSchema));
}
