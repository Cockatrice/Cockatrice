export enum StatusEnum {
  DISCONNECTED,
  CONNECTING,
  CONNECTED,
  LOGGING_IN,
  LOGGED_IN,
  /** Separated from sequential states to reserve room for future connection phases. */
  RECONNECTING = 50,
  /** High sentinel value — marks the terminal "tearing down" state that must not collide with future states added above. */
  DISCONNECTING = 99,
}
