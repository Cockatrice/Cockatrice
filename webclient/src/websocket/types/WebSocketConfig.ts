import type {
  GameEventContext,
  Response_Login,
  Response,
  Event_RoomSay,
  ResponseMap,
  RoomEventMap,
} from '@app/generated';

// `KeyOf<ResponseMap, Response_Login>` resolves to `'Response_Login'`.
export type KeyOf<Map, V> = { [K in keyof Map]: Map[K] extends V ? K : never }[keyof Map];

export interface GameEventMeta {
  gameId: number;
  playerId: number;
  context: GameEventContext | null;
  secondsElapsed: number;
  forcedByJudge: number;
}

export interface WebSocketSessionResponseOverrides extends ResponseMap {
  Response_Login: Response_Login & { hashedPassword?: string };
  Response: Response & { host: string; port: string; userName: string };
}

export interface WebSocketRoomResponseOverrides extends RoomEventMap {
  Event_RoomSay: Event_RoomSay & { timeReceived: number };
}
