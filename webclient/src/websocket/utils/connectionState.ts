import type { WebSocketConnectOptions } from '../interfaces/ConnectOptions';

let pendingOptions: WebSocketConnectOptions | null = null;

export function setPendingOptions(options: WebSocketConnectOptions) {
  pendingOptions = options;
}

export function consumePendingOptions(): WebSocketConnectOptions | null {
  const opts = pendingOptions;
  pendingOptions = null;
  return opts;
}
