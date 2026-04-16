import { create } from '@bufbuild/protobuf';
import { Command_DumpZone_ext, Command_DumpZoneSchema, type DumpZoneParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function dumpZone(gameId: number, params: DumpZoneParams): void {
  WebClient.instance.protobuf.sendGameCommand(gameId, Command_DumpZone_ext, create(Command_DumpZoneSchema, params));
}
