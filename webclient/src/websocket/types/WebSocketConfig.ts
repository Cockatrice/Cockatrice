import type {
  GameEventContext,
  Event_RoomSay,
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

export interface WebSocketRoomResponseOverrides extends RoomEventMap {
  Event_RoomSay: Event_RoomSay & { timeReceived: number };
}
