import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function accountEdit(passwordCheck: string, realName?: string, email?: string, country?: string): void {
  BackendService.sendSessionCommand('Command_AccountEdit', { passwordCheck, realName, email, country }, {
    onSuccess: () => {
      SessionPersistence.accountEditChanged(realName, email, country);
    },
  });
}
