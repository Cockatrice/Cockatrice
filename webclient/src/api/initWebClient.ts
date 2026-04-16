import {
  WebClient,
  SessionEvents,
  RoomEvents,
  GameEvents,
  SessionCommands,
} from '@app/websocket';
import type { WebClientConfig } from '@app/websocket';

import { createWebClientResponse } from './response';

export function initWebClient(): void {
  const response = createWebClientResponse();

  const config: WebClientConfig = {
    response,
    sessionEvents: SessionEvents,
    roomEvents: RoomEvents,
    gameEvents: GameEvents,
    keepAliveFn: (cb) => SessionCommands.ping(cb),
  };

  new WebClient(config);
}
