import { App } from '@app/types';
import { WebClient } from '../../WebClient';
export function updateStatus(status: App.StatusEnum, description: string): void {
  WebClient.instance.response.session.updateStatus(status, description);

  WebClient.instance.updateStatus(status);
}
