import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function accountPassword(oldPassword: string, newPassword: string, hashedNewPassword: string): void {
  BackendService.sendSessionCommand('Command_AccountPassword', { oldPassword, newPassword, hashedNewPassword }, {
    onSuccess: () => {
      SessionPersistence.accountPasswordChange();
    },
  });
}
