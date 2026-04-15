import type {
  Event_RoomSay,
  GameEventContext,
  ServerInfo_ChatMessage,
  ServerInfo_Game,
  ServerInfo_Room,
} from '@app/generated';

import { WebSocketConnectReason } from './server';

// ── Domain model types (proto types extended with client-side fields) ─────────

export type Game = ServerInfo_Game & {
  gameType: string;
};

export interface GametypeMap { [index: number]: string }

export type Room = ServerInfo_Room & {
  gametypeMap: GametypeMap;
  gameList: Game[];
  order: number;
};

export type Message = Event_RoomSay & {
  timeReceived: number;
};

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

// ── Connect options ───────────────────────────────────────────────────────────
// Each variant is the enriched input for one session flow: the network
// transport fields (host/port) + the subset of proto Command_* fields the UI
// actually produces (user-entered credentials, tokens, email, etc.) + a
// `reason` discriminator so the websocket layer can route.
//
// Hand-written instead of `MessageInitShape<typeof Command_XSchema> & ...`
// because MessageInitShape is a `Message<T> | { initShape }` union which
// collapses to the Message branding when intersected, requiring `$typeName`
// on literals. Keep these in sync with the corresponding proto command by
// convention; fields here map 1:1 to Command_* members.

interface ConnectTransport {
  host: string;
  port: string;
  keepalive?: number;
  autojoinrooms?: boolean;
  clientid?: string;
}

export interface LoginConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.LOGIN;
  userName: string;
  password?: string;
  hashedPassword?: string;
}

export interface RegisterConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.REGISTER;
  userName: string;
  password: string;
  email: string;
  country: string;
  realName: string;
}

export interface ActivateConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.ACTIVATE_ACCOUNT;
  userName: string;
  token: string;
  /** Plaintext password carried through so post-activation auto-login can hash it. */
  password?: string;
}

export interface PasswordResetRequestConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.PASSWORD_RESET_REQUEST;
  userName: string;
}

export interface PasswordResetChallengeConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.PASSWORD_RESET_CHALLENGE;
  userName: string;
  email: string;
}

export interface PasswordResetConnectOptions extends ConnectTransport {
  reason: WebSocketConnectReason.PASSWORD_RESET;
  userName: string;
  token: string;
  newPassword: string;
}

/**
 * Test connection has no proto command — it just opens and closes a socket to
 * verify reachability.
 */
export interface TestConnectionOptions extends ConnectTransport {
  reason: WebSocketConnectReason.TEST_CONNECTION;
}

export type WebSocketConnectOptions =
  | LoginConnectOptions
  | RegisterConnectOptions
  | ActivateConnectOptions
  | PasswordResetRequestConnectOptions
  | PasswordResetChallengeConnectOptions
  | PasswordResetConnectOptions
  | TestConnectionOptions;

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
