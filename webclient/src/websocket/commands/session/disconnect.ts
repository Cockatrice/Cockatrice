import { WebClient } from '../../WebClient';

export function disconnect(): void {
  WebClient.instance.disconnect();
}
