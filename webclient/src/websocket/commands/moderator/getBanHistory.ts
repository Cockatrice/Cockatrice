import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function getBanHistory(userName: string): void {
  BackendService.sendModeratorCommand('Command_GetBanHistory', { userName }, {
    responseName: 'Response_BanHistory',
    onSuccess: (response) => {
      ModeratorPersistence.banHistory(userName, response.banList);
    },
  });
}
