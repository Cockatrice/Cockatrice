import { waitForServatriceReady } from './helpers/servatrice';

export default async function globalSetup(): Promise<void> {
  // The compose entrypoint sleeps 10s waiting for MySQL. On cold CI runs
  // building the image on top of that can push the first WS frame well past
  // 30s, so poll for up to 60s before giving up.
  await waitForServatriceReady(60_000);
}
