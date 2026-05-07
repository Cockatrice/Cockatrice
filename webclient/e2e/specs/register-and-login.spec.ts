import { test, expect } from '@playwright/test';
import { seedLocalHostAndReload } from '../helpers/seedLocalHost';
import { generateUniqueUser, loginViaUi, registerViaUi } from '../helpers/testUser';

test('a fresh user can register, then log in against local servatrice', async ({ page }) => {
  const user = generateUniqueUser();

  // Seed IndexedDB so the Login form auto-selects our Local E2E host instead
  // of one of the production defaults. After this returns the app has been
  // reloaded; AppShell uses MemoryRouter, so the address bar never reflects
  // the active route — assert on DOM markers unique to each screen instead.
  await seedLocalHostAndReload(page);
  await expect(page.locator('.login-content')).toBeVisible();

  await registerViaUi(page, user);
  await loginViaUi(page, user);

  await expect(page.locator('.serverRoomWrapper')).toBeVisible();
});
