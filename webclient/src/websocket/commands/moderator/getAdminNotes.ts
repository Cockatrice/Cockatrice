import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function getAdminNotes(userName: string): void {
  BackendService.sendModeratorCommand('Command_GetAdminNotes', { userName }, {
    responseName: 'Response_GetAdminNotes',
    onSuccess: (response) => {
      ModeratorPersistence.getAdminNotes(userName, response.notes);
    },
  });
}
