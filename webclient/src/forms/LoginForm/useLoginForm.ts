import { useState } from 'react';
import { useFormState } from 'react-final-form';

import { LoadingState, useKnownHosts, useSettings } from '@app/hooks';
import { HostDTO } from '@app/services';

export interface LoginFormBody {
  useStoredPasswordLabel: boolean;
  setUseStoredPasswordLabel: (v: boolean) => void;
  onSelectedHostChange: (host: HostDTO | undefined) => void;
  onUserNameChange: (userName: string | undefined) => void;
  onRememberChange: (checked: boolean) => void;
  onUserToggleAutoConnect: (checked: boolean, fieldOnChange: (v: boolean) => void) => void;
  passwordFieldBlur: () => void;
}

// `FormApi` import from react-final-form is broken at the type level on this
// branch (baseline TS error). Only `form.change` is used here.
interface MinimalFormApi {
  change: (name: string, value: unknown) => void;
}

export function useLoginFormBody(form: MinimalFormApi): LoginFormBody {
  const settings = useSettings();
  const hosts = useKnownHosts();
  const { values } = useFormState();

  const selectedHost = hosts.status === LoadingState.READY ? hosts.value?.selectedHost : undefined;

  const [useStoredPasswordLabel, setUseStoredPasswordLabel] = useState(false);
  const [storedHashInvalidated, setStoredHashInvalidated] = useState(false);

  const canUseStoredPassword = (remember: boolean, password: string | undefined) =>
    Boolean(remember && selectedHost?.hashedPassword && !password && !storedHashInvalidated);

  const togglePasswordLabel = (on: boolean) => setUseStoredPasswordLabel(on);

  // @critical Host-sync must not touch autoConnect — app-level setting, not per-host.
  const onSelectedHostChange = (host: HostDTO | undefined) => {
    if (!host) {
      return;
    }
    form.change('userName', host.userName ?? '');
    form.change('password', '');
    form.change('remember', Boolean(host.remember));
    setStoredHashInvalidated(false);
    togglePasswordLabel(Boolean(host.remember && host.hashedPassword));
  };

  const onUserNameChange = (userName: string | undefined) => {
    const fieldChanged = selectedHost?.userName?.toLowerCase() !== userName?.toLowerCase();
    if (canUseStoredPassword(values.remember, values.password) && fieldChanged) {
      setStoredHashInvalidated(true);
    }
  };

  const onRememberChange = (checked: boolean) => {
    // @critical Writes form-only, never to persisted setting — "remember" toggle isn't a preference edit.
    if (!checked && values.autoConnect) {
      form.change('autoConnect', false);
    }

    togglePasswordLabel(canUseStoredPassword(checked, values.password));
  };

  // @critical Only persist-path for autoConnect; wired to native onChange, not <OnChange>,
  // to avoid leaking form.change() writes into Dexie.
  const onUserToggleAutoConnect = (checked: boolean, fieldOnChange: (v: boolean) => void) => {
    fieldOnChange(checked);

    if (settings.status === LoadingState.READY) {
      void settings.update({ autoConnect: checked });
    }

    if (checked && !values.remember) {
      form.change('remember', true);
    }
  };

  const passwordFieldBlur = () =>
    togglePasswordLabel(canUseStoredPassword(values.remember, values.password));

  return {
    useStoredPasswordLabel,
    setUseStoredPasswordLabel,
    onSelectedHostChange,
    onUserNameChange,
    onRememberChange,
    onUserToggleAutoConnect,
    passwordFieldBlur,
  };
}
