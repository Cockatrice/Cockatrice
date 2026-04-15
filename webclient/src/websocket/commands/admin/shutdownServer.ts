import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';

export function shutdownServer(reason: string, minutes: number): void {
  BackendService.sendAdminCommand('Command_ShutdownServer', { reason, minutes }, {
    onSuccess: () => {
      AdminPersistence.shutdownServer();
    },
  });
}
