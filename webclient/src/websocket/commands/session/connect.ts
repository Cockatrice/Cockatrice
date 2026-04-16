import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../interfaces/WebClientConfig';

export function connect(target: ConnectTarget): void {
  WebClient.instance.connect(target);
}

export function testConnect(target: ConnectTarget): void {
  WebClient.instance.testConnect(target);
}
