import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';

export function reloadConfig(): void {
  BackendService.sendAdminCommand('Command_ReloadConfig', {}, {
    onSuccess: () => {
      AdminPersistence.reloadConfig();
    },
  });
}
