import { useEffect } from 'react';

import type { HostDTO } from '@app/services';

import { getKnownHosts } from './useKnownHosts';
import { getSettings } from './useSettings';

export interface LoginFormValues {
  userName: string;
  password?: string;
  selectedHost: HostDTO;
  remember: boolean;
  autoConnect?: boolean;
}

// Auto-login is a *startup* concern — the persisted preference is consulted
// once per JS session, after both stores have loaded. A logout within the
// same session is an explicit user action; returning to /login should not
// re-auto-connect (matches Cockatrice desktop behaviour). The flag is
// module-scope so it persists across Login remounts and is naturally reset
// on page refresh, which is the one time we do want another try.
//
// The flag tracks whether the *check* has run, not whether it *fired* — a
// manual first login followed by a logout must not re-trigger auto-login
// either, so the outcome of the check is irrelevant; only that it happened.
//
// Exported as a mutable object (rather than a bare `let`) so integration
// tests can reset `startupCheckRan = false` between scenarios without
// resorting to `vi.resetModules`, which is prohibitively slow in the full
// suite. Production code only writes the flag from inside the effect.
export const autoLoginSession = { startupCheckRan: false };

// Deliberately does NOT subscribe to the settings / known-hosts stores —
// user actions that change those stores (ticking the auto-connect checkbox,
// picking a different host) are preference edits, not "log in now" signals.
export function useAutoLogin(
  onLogin: (values: LoginFormValues) => void,
  connectionAttemptMade: boolean,
): void {
  useEffect(() => {
    if (autoLoginSession.startupCheckRan) {
      return;
    }
    if (connectionAttemptMade) {
      return;
    }

    let cancelled = false;

    Promise.all([getSettings(), getKnownHosts()]).then(([settings, hosts]) => {
      if (cancelled || autoLoginSession.startupCheckRan) {
        return;
      }
      autoLoginSession.startupCheckRan = true;

      if (!settings.autoConnect) {
        return;
      }
      const { selectedHost } = hosts;
      if (!selectedHost?.remember || !selectedHost?.hashedPassword) {
        return;
      }

      onLogin({
        selectedHost,
        userName: selectedHost.userName ?? '',
        remember: true,
        password: '',
      });
    });

    return () => {
      cancelled = true;
    };
  }, [connectionAttemptMade, onLogin]);
}
