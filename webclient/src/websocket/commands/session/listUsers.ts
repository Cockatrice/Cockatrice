import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function listUsers(): void {
  BackendService.sendSessionCommand('Command_ListUsers', {}, {
    responseName: 'Response_ListUsers',
    onSuccess: (response) => {
      SessionPersistence.updateUsers(response.userList);
    },
  });
}
