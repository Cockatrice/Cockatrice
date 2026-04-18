// Clears every table the services suite touches so each test starts from
// empty storage. Dexie is a real singleton, the database a real (fake-
// indexeddb) instance, so state leaks between tests otherwise.

import { dexieService } from '@app/services';

export async function resetDexie(): Promise<void> {
  await Promise.all([
    dexieService.settings.clear(),
    dexieService.hosts.clear(),
  ]);
}
