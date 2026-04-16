export * from './commands';
export * from './interfaces';

export { WebClient } from './WebClient';
export { StatusEnum } from './interfaces/StatusEnum';
export type { WebClientConfig, ConnectTarget } from './interfaces/WebClientConfig';
export type {
  KeyOf,
  GameEventMeta,
  WebSocketSessionResponseOverrides,
  WebSocketRoomResponseOverrides,
} from './interfaces/WebSocketConfig';

export { SessionEvents } from './events/session';
export { RoomEvents } from './events/room';
export { GameEvents } from './events/game';

export { generateSalt, passwordSaltSupported, hashPassword } from './utils';

export { WebSocketConnectReason } from './interfaces/ConnectOptions';
export type {
  LoginConnectOptions,
  RegisterConnectOptions,
  ActivateConnectOptions,
  PasswordResetRequestConnectOptions,
  PasswordResetChallengeConnectOptions,
  PasswordResetConnectOptions,
  TestConnectionOptions,
  WebSocketConnectOptions,
} from './interfaces/ConnectOptions';

export { setPendingOptions, consumePendingOptions } from './utils/connectionState';
