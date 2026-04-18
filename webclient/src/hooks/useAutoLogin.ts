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

export const autoLoginGate = { hasChecked: false };

export function useAutoLogin(
  onLogin: (values: LoginFormValues) => void,
  connectionAttemptMade: boolean,
): void {
  useEffect(() => {
    if (autoLoginGate.hasChecked) {
      return;
    }
    if (connectionAttemptMade) {
      return;
    }

    let cancelled = false;

    Promise.all([getSettings(), getKnownHosts()]).then(([settings, hosts]) => {
      if (cancelled || autoLoginGate.hasChecked) {
        return;
      }
      autoLoginGate.hasChecked = true;

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
