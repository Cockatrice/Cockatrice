import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function changeZoneProperties(gameId: number, params: Data.ChangeZonePropertiesParams): void {
  WebClient.instance.protobuf.sendGameCommand(
    gameId,
    Data.Command_ChangeZoneProperties_ext,
    create(Data.Command_ChangeZonePropertiesSchema, params)
  );
}
