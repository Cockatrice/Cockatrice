import { WebSocket } from 'ws';

// The e2e docker-compose.yml publishes servatrice on localhost:4748.
export const E2E_WS_URL = 'ws://localhost:4748';

// Opens a WS to servatrice and resolves once the server sends *anything* back
// (ServerIdentification is the first frame). Used by global-setup to wait out
// the `sleep 10` in the docker-compose entrypoint before any spec runs.
export async function waitForServatriceReady(timeoutMs = 60_000): Promise<void> {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    const ok = await tryConnect(E2E_WS_URL).catch(() => false);
    if (ok) {
      return;
    }
    await sleep(1000);
  }
  throw new Error(`servatrice did not become ready at ${E2E_WS_URL} within ${timeoutMs}ms`);
}

function tryConnect(url: string): Promise<boolean> {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(url);
    const done = (value: boolean, err?: Error) => {
      try { ws.removeAllListeners(); ws.close(); } catch { /* ignore */ }
      err ? reject(err) : resolve(value);
    };
    ws.once('message', () => done(true));
    ws.once('error', (err) => done(false, err));
    ws.once('close', () => done(false));
  });
}

function sleep(ms: number): Promise<void> {
  return new Promise((r) => setTimeout(r, ms));
}
