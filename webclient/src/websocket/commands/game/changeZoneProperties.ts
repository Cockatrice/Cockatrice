import { create } from '@bufbuild/protobuf';
import { Command_ChangeZoneProperties_ext, Command_ChangeZonePropertiesSchema, type ChangeZonePropertiesParams } from '@app/generated';
import { WebClient } from '../../WebClient';

export function changeZoneProperties(gameId: number, params: ChangeZonePropertiesParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Command_ChangeZoneProperties_ext,
    create(Command_ChangeZonePropertiesSchema, params)
  );
}
