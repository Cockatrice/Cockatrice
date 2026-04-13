import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DumpZoneSchema, Command_DumpZone_ext } from 'generated/proto/command_dump_zone_pb';
import { DumpZoneParams } from 'types';

export function dumpZone(gameId: number, params: DumpZoneParams): void {
  BackendService.sendGameCommand(gameId, Command_DumpZone_ext, create(Command_DumpZoneSchema, params));
}
