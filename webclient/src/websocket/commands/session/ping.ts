import { BackendService } from '../../services/BackendService';

export function ping(pingReceived: Function): void {
  BackendService.sendSessionCommand('Command_Ping', {}, {
    onResponse: (raw) => pingReceived(raw),
  });
}
