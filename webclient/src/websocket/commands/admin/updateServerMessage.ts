import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';

export function updateServerMessage(): void {
  BackendService.sendAdminCommand('Command_UpdateServerMessage', {}, {
    onSuccess: () => {
      AdminPersistence.updateServerMessage();
    },
  });
}
