import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  BackendService.sendModeratorCommand('Command_GetWarnList', { modName, userName, userClientid }, {
    responseName: 'Response_WarnList',
    onSuccess: (response) => {
      ModeratorPersistence.warnListOptions(response.warning);
    },
  });
}
