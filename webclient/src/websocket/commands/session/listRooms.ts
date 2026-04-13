import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ListRooms_ext, Command_ListRoomsSchema } from 'generated/proto/session_commands_pb';

export function listRooms(): void {
  BackendService.sendSessionCommand(Command_ListRooms_ext, create(Command_ListRoomsSchema));
}
