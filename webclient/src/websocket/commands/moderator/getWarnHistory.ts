import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function getWarnHistory(userName: string): void {
  BackendService.sendModeratorCommand('Command_GetWarnHistory', { userName }, {
    responseName: 'Response_WarnHistory',
    onSuccess: (response) => {
      ModeratorPersistence.warnHistory(userName, response.warnList);
    },
  });
}
