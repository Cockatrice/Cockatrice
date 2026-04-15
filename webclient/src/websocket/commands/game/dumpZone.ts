import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { Data } from '@app/types';

export function dumpZone(gameId: number, params: Data.DumpZoneParams): void {
  webClient.protobuf.sendGameCommand(gameId, Data.Command_DumpZone_ext, create(Data.Command_DumpZoneSchema, params));
}
