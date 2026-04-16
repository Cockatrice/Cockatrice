import type {
  Event_RoomSay,
  GameEventContext,
  ServerInfo_Arrow,
  ServerInfo_Card,
  ServerInfo_ChatMessage,
  ServerInfo_Counter,
  ServerInfo_Game,
  ServerInfo_PlayerProperties,
  ServerInfo_Room,
  ServerInfo_User,
} from '@app/generated';

// ── Domain model types (composition: raw proto + client-side fields) ──────────
//
// `info` holds the proto snapshot verbatim. Normalized/client-only fields
// live as siblings. For `Room`, the repeated collections on `info`
// (gameList, userList, gametypeList) are the *wire snapshot* from the last
// full update — they become stale after subsequent events. Always read from
// the normalized `games`, `users`, and `gametypeMap` fields.

export interface GametypeMap { [index: number]: string }

/** Room directory listing — composition of raw proto with normalized collections. */
export interface Room {
  info: ServerInfo_Room;
  gametypeMap: GametypeMap;
  /** Server-determined display order from the UPDATE_ROOMS sequence. */
  order: number;
  games: { [gameId: number]: Game };
  users: { [userName: string]: ServerInfo_User };
}

/** Room directory game listing — composition of raw proto with cached gameType. */
export interface Game {
  info: ServerInfo_Game;
  /** Cached display string resolved from the owning room's gametypeMap at ingest. */
  gameType: string;
}

export type Message = Event_RoomSay & {
  timeReceived: number;
};

// ── Active game runtime state (game slice) ───────────────────────────────────
//
// Composition pattern: the raw proto from Event_GameJoined is stored verbatim
// on `info`. Fields that evolve via in-game events live at the top level.
//
// Convention: `info` is the wire snapshot taken at join time. Fields with a
// proto twin (e.g. `started`) diverge after the first event update — always
// read the top-level field for "current value"; `info.*` is the initial
// server snapshot only.

export interface GameEntry {
  info: ServerInfo_Game;

  // From the Event_GameJoined wrapper (not on ServerInfo_Game itself).
  hostId: number;
  localPlayerId: number;
  spectator: boolean;
  judge: boolean;
  resuming: boolean;

  // Client-tracked runtime state, updated by game events.
  started: boolean;
  activePlayerId: number;
  activePhase: number;
  secondsElapsed: number;
  reversed: boolean;

  players: { [playerId: number]: PlayerEntry };
  messages: GameMessage[];
}

/** Normalized from ServerInfo_Player — keyed collections for O(1) lookup. */
export interface PlayerEntry {
  properties: ServerInfo_PlayerProperties;
  deckList: string;
  /** Zones keyed by zone name (e.g. "hand", "deck", "table"). */
  zones: { [zoneName: string]: ZoneEntry };
  /** Player-level counters (e.g. life) keyed by counter id. */
  counters: { [counterId: number]: ServerInfo_Counter };
  /** Arrows keyed by arrow id. */
  arrows: { [arrowId: number]: ServerInfo_Arrow };
}

/**
 * Normalized from ServerInfo_Zone — cards indexed by id for O(1) mutation,
 * with `order` preserving display sequence. Iterate via `order.map(id => byId[id])`.
 */
export interface ZoneEntry {
  name: string;
  /** ZoneType enum value (0=Private, 1=Public, 2=Hidden). */
  type: number;
  withCoords: boolean;
  /** Authoritative card count. For hidden zones this may exceed `order.length`. */
  cardCount: number;
  /** Card ids in display order. */
  order: number[];
  /** Card lookup by id. */
  byId: { [cardId: number]: ServerInfo_Card };
  alwaysRevealTopCard: boolean;
  alwaysLookAtTopCard: boolean;
}

export interface GameMessage {
  playerId: number;
  message: string;
  timeReceived: number;
}

/**
 * Passed to every game event handler alongside the event payload.
 * Contains per-container metadata from GameEventContainer.
 * Not stored in Redux — transient routing metadata only.
 */
export interface GameEventMeta {
  gameId: number;
  playerId: number;
  /** Raw protobuf GameEventContext object. Not stored in Redux. */
  context: GameEventContext | null;
  secondsElapsed: number;
  /** Proto type is uint32. Non-zero means the action was forced by a judge. */
  forcedByJudge: number;
}

export interface LogGroups {
  room: ServerInfo_ChatMessage[];
  game: ServerInfo_ChatMessage[];
  chat: ServerInfo_ChatMessage[];
}

// ── Connect options (re-exported from @app/websocket) ────────────────────────
// Source of truth lives in src/websocket/connectOptions.ts. Re-exported here
// so UI code can use the Enriched.* namespace without importing @app/websocket.

export type {
  LoginConnectOptions,
  RegisterConnectOptions,
  ActivateConnectOptions,
  PasswordResetRequestConnectOptions,
  PasswordResetChallengeConnectOptions,
  PasswordResetConnectOptions,
  TestConnectionOptions,
  WebSocketConnectOptions,
} from '@app/websocket';

/**
 * Context preserved through the ACCOUNT_AWAITING_ACTIVATION signal so the
 * activation dialog can resubmit against the same host/user without re-entering them.
 */
export interface PendingActivationContext {
  host: string;
  port: string;
  userName: string;
}

/**
 * Payload for the LOGIN_SUCCESSFUL signal. Only carries what the UI needs to
 * persist into the selected host record (hashedPassword for "remember me").
 */
export interface LoginSuccessContext {
  hashedPassword?: string;
}
