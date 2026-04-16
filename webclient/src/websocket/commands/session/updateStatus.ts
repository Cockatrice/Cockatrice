import { StatusEnum } from '../../StatusEnum';
import { WebClient } from '../../WebClient';

export function updateStatus(status: StatusEnum, description: string): void {
  WebClient.instance.response.session.updateStatus(status, description);
  WebClient.instance.updateStatus(status);
}
