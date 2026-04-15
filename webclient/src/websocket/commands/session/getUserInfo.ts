import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function getUserInfo(userName: string): void {
  BackendService.sendSessionCommand('Command_GetUserInfo', { userName }, {
    responseName: 'Response_GetUserInfo',
    onSuccess: (response) => {
      SessionPersistence.getUserInfo(response.userInfo);
    },
  });
}
