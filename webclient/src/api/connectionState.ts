import type { Enriched } from '@app/types';

let pendingOptions: Enriched.WebSocketConnectOptions | null = null;

export function setPendingOptions(options: Enriched.WebSocketConnectOptions) {
  pendingOptions = options;
}

export function consumePendingOptions(): Enriched.WebSocketConnectOptions | null {
  const opts = pendingOptions;
  pendingOptions = null;
  return opts;
}
