import type { Page } from '@playwright/test';
import { E2E_WS_URL } from './servatrice';

// Points the running webclient at the local servatrice without touching any
// webclient source. Strategy: navigate to the app once (which lets Dexie open
// the Webatrice IndexedDB at schema v1 and seed DefaultHosts), then wipe the
// `hosts` store and insert a single "Local E2E" row with lastSelected=true,
// then reload so useKnownHosts re-reads. On a fresh Playwright context the
// IDB is empty, so our seed is the only known host after reload — the Login
// form auto-selects it and fires its test-connection against :4748.
export async function seedLocalHostAndReload(page: Page, wsUrl: string = E2E_WS_URL): Promise<void> {
  const parsed = new URL(wsUrl);
  const host = parsed.hostname;
  const port = parsed.port || '4748';

  await page.goto('/');

  await page.evaluate(async ({ host, port }) => {
    await new Promise<void>((resolve, reject) => {
      const open = indexedDB.open('Webatrice');
      open.onerror = () => reject(open.error ?? new Error('failed to open Webatrice DB'));
      open.onsuccess = () => {
        const db = open.result;
        if (!db.objectStoreNames.contains('hosts')) {
          db.close();
          reject(new Error('hosts object store missing — did Dexie finish opening?'));
          return;
        }
        const tx = db.transaction('hosts', 'readwrite');
        const store = tx.objectStore('hosts');
        store.clear();
        store.add({
          name: 'Local E2E',
          host,
          port,
          localHost: host,
          localPort: port,
          editable: false,
          lastSelected: true,
        });
        tx.oncomplete = () => { db.close(); resolve(); };
        tx.onerror = () => { db.close(); reject(tx.error ?? new Error('hosts write failed')); };
      };
    });
  }, { host, port });

  await page.reload();
}
