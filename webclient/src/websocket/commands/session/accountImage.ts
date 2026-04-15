import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function accountImage(image: Uint8Array): void {
  BackendService.sendSessionCommand('Command_AccountImage', { image }, {
    onSuccess: () => {
      SessionPersistence.accountImageChanged(image);
    },
  });
}
