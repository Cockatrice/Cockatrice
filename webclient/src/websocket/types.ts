import type {
  GameEventContext,
  Response_Login,
  Response,
  Event_RoomSay,
  ResponseMap,
  RoomEventMap,
} from '@app/generated';

// ── KeyOf utility ────────────────────────────────────────────────────────────
// Derives a type map key from a generated type. Allows interface methods to
// reference generated types instead of hardcoded string keys.
//
//   T[KeyOf<ResponseMap, Response_Login>]
//   ↓ resolves to ↓
//   T['Response_Login']

export type KeyOf<Map, V> = { [K in keyof Map]: Map[K] extends V ? K : never }[keyof Map];

// ── GameEventMeta ────────────────────────────────────────────────────────────
// Per-container metadata passed to every game event handler alongside the
// event payload. Constructed by ProtobufService.processGameEvent from the
// GameEventContainer fields. Structurally identical to Enriched.GameEventMeta.

export interface GameEventMeta {
  gameId: number;
  playerId: number;
  context: GameEventContext | null;
  secondsElapsed: number;
  forcedByJudge: number;
}

// ── Websocket-layer enrichments ──────────────────────────────────────────────
// Protocol-level enrichments of proto types — these are websocket concerns,
// not app concerns. Used as the DEFAULT generic on the response interfaces.

export interface WebSocketSessionResponseOverrides extends ResponseMap {
  Response_Login: Response_Login & { hashedPassword?: string };
  Response: Response & { host: string; port: string; userName: string };
}

export interface WebSocketRoomResponseOverrides extends RoomEventMap {
  Event_RoomSay: Event_RoomSay & { timeReceived: number };
}
