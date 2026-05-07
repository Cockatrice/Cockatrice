import { randomBytes } from 'node:crypto';
import { expect, type Page } from '@playwright/test';

export interface TestUser {
  userName: string;
  password: string;
}

// Servatrice [users] rules (see webclient/e2e/docker/servatrice-e2e.ini):
//   - 4-12 chars, lowercase/uppercase/numerics + "_.-", no leading punctuation
//   - "admin" is blacklisted
// "e2e_" + 6 hex chars = 10 chars — fits comfortably and is unique per call.
export function generateUniqueUser(): TestUser {
  return {
    userName: 'e2e_' + randomBytes(3).toString('hex'),
    password: 'TestPw123!',
  };
}

export async function registerViaUi(page: Page, user: TestUser): Promise<void> {
  await page.getByRole('button', { name: /register/i }).first().click();

  const dialog = page.getByRole('dialog');
  await expect(dialog).toBeVisible();

  await dialog.locator('input[name="userName"]').fill(user.userName);
  await dialog.locator('input[name="password"]').fill(user.password);
  await dialog.locator('input[name="passwordConfirm"]').fill(user.password);

  await dialog.getByRole('button', { name: /register/i }).click();

  // With requireemailactivation=false the server returns RegistrationAccepted
  // and the dialog closes. If activation were required an AccountActivation
  // dialog would replace it — that's out of scope for this spec.
  await expect(dialog).toBeHidden();
}

export async function loginViaUi(page: Page, user: TestUser): Promise<void> {
  await page.locator('input[name="userName"]').fill(user.userName);
  await page.locator('input[name="password"]').fill(user.password);

  const loginButton = page.locator('button.loginForm-submit');
  // The Login button is gated on a successful test-connection (fired
  // automatically when the host loads). Wait it out rather than racing it.
  await expect(loginButton).toBeEnabled();
  await loginButton.click();
}
