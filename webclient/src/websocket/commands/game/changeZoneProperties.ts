import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_ChangeZonePropertiesSchema, Command_ChangeZoneProperties_ext } from 'generated/proto/command_change_zone_properties_pb';
import { ChangeZonePropertiesParams } from 'types';

export function changeZoneProperties(gameId: number, params: ChangeZonePropertiesParams): void {
  BackendService.sendGameCommand(gameId, Command_ChangeZoneProperties_ext, create(Command_ChangeZonePropertiesSchema, params));
}
