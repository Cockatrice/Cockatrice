import type {
  RegistryEntry,
  SessionEvent,
  RoomEvent,
  GameEvent,
  Event_ServerIdentification,
} from '@app/generated';

import type { GameEventMeta } from './types';
import type { IWebClientResponse } from './interfaces';

export interface ConnectTarget {
  host: string;
  port: string;
}

export interface WebClientConfig {
  response: IWebClientResponse;

  onServerIdentified(info: Event_ServerIdentification): void;

  sessionEvents: RegistryEntry<unknown, SessionEvent>[];
  roomEvents: RegistryEntry<unknown, RoomEvent, RoomEvent>[];
  gameEvents: RegistryEntry<unknown, GameEvent, GameEventMeta>[];

  keepAliveFn(pingReceived: () => void): void;
}
