import { BackendService } from '../../services/BackendService';

export function listRooms(): void {
  BackendService.sendSessionCommand('Command_ListRooms', {}, {});
}
