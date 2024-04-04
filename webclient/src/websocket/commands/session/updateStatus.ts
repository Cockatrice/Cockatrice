import { StatusEnum } from 'types';
import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';

export function updateStatus(status: StatusEnum, description: string): void {
  SessionPersistence.updateStatus(status, description);

  webClient.updateStatus(status);
}
