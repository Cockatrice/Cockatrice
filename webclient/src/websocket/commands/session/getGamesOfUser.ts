import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function getGamesOfUser(userName: string): void {
  BackendService.sendSessionCommand('Command_GetGamesOfUser', { userName }, {
    responseName: 'Response_GetGamesOfUser',
    onSuccess: (response) => {
      SessionPersistence.getGamesOfUser(userName, response);
    },
  });
}
