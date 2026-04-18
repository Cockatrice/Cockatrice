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
