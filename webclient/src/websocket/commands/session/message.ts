import { BackendService } from '../../services/BackendService';

export function message(userName: string, message: string): void {
  BackendService.sendSessionCommand('Command_Message', { userName, message }, {});
}
