import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function updateAdminNotes(userName: string, notes: string): void {
  BackendService.sendModeratorCommand('Command_UpdateAdminNotes', { userName, notes }, {
    onSuccess: () => {
      ModeratorPersistence.updateAdminNotes(userName, notes);
    },
  });
}
