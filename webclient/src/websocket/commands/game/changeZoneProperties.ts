import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { Command_ChangeZonePropertiesSchema, Command_ChangeZoneProperties_ext } from 'generated/proto/command_change_zone_properties_pb';
import { ChangeZonePropertiesParams } from 'types';

export function changeZoneProperties(gameId: number, params: ChangeZonePropertiesParams): void {
  webClient.protobuf.sendGameCommand(gameId, Command_ChangeZoneProperties_ext, create(Command_ChangeZonePropertiesSchema, params));
}
