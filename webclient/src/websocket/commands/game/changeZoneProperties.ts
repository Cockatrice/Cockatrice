import { BackendService } from '../../services/BackendService';
import { ChangeZonePropertiesParams } from 'types';

export function changeZoneProperties(gameId: number, params: ChangeZonePropertiesParams): void {
  BackendService.sendGameCommand(gameId, 'Command_ChangeZoneProperties', params);
}
