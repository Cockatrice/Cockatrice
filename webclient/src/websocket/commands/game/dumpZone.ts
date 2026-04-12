import { BackendService } from '../../services/BackendService';
import { DumpZoneParams } from 'types';

export function dumpZone(gameId: number, params: DumpZoneParams): void {
  BackendService.sendGameCommand(gameId, 'Command_DumpZone', params);
}
