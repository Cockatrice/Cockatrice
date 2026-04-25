// Known hosts fall into two shapes:
//   1. Direct endpoint: `mtg.chickatrice.net` + `4748`  →  ws://mtg.chickatrice.net:4748
//   2. Reverse-proxied endpoint with an nginx route baked into the host:
//      `server.cockatrice.us/servatrice` + `4748`       →  wss://server.cockatrice.us/servatrice
//
// For shape (2) the `port` field is a dev-convenience value for direct
// connections — on the public TLS endpoint the path is routed via nginx on the
// default 443, NOT on `:4748`. A naive `${protocol}://${host}:${port}` produces
// `wss://server.cockatrice.us/servatrice:4748` which browsers parse as
// host=server.cockatrice.us, port=default, path=/servatrice:4748 — bypassing
// the proxy and failing the WS upgrade.
export function buildWebSocketUrl(
  protocol: 'ws' | 'wss',
  host: string,
  port: string | number,
): string {
  if (host.includes('/')) {
    return `${protocol}://${host}`;
  }
  return `${protocol}://${host}:${port}`;
}
