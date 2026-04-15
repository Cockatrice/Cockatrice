import { App } from '@app/types';
import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function updateStatus(status: App.StatusEnum, description: string): void {
  SessionPersistence.updateStatus(status, description);

  webClient.updateStatus(status);
}
