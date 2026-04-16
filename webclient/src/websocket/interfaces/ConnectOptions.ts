import type { ConnectTarget } from './WebClientConfig';

export enum WebSocketConnectReason {
  LOGIN,
  REGISTER,
  ACTIVATE_ACCOUNT,
  PASSWORD_RESET_REQUEST,
  PASSWORD_RESET_CHALLENGE,
  PASSWORD_RESET,
  TEST_CONNECTION,
}

// ── Connect options ───────────────────────────────────────────────────────────
// Each variant is the enriched input for one session flow: the network
// transport fields (host/port) + the subset of proto Command_* fields the UI
// actually produces (user-entered credentials, tokens, email, etc.) + a
// `reason` discriminator so the websocket layer can route.

interface ConnectTransport extends ConnectTarget {
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
 * Test connection has no proto command -- it just opens and closes a socket to
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
