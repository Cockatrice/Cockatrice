import React, { useEffect, useState } from 'react';
import { Form, Field, useFormState, FormApi } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Checkbox from '@mui/material/Checkbox';
import FormControlLabel from '@mui/material/FormControlLabel';

import { CheckboxField, InputField, KnownHosts } from '@app/components';
import { LoadingState, useKnownHosts, useSettings } from '@app/hooks';

import './LoginForm.css';

interface LoginFormProps {
  onSubmit: (values: any) => void;
  disableSubmitButton: boolean;
  onResetPassword: () => void;
}

interface LoginFormBodyProps extends LoginFormProps {
  form: FormApi;
  handleSubmit: (event?: React.SyntheticEvent) => void;
}

const LoginFormBody = ({
  form,
  handleSubmit,
  disableSubmitButton,
  onResetPassword,
}: LoginFormBodyProps) => {
  const { t } = useTranslation();
  const PASSWORD_LABEL = t('Common.label.password');
  const STORED_PASSWORD_LABEL = `* ${t('LoginForm.label.savedPassword')} *`;

  const settings = useSettings();
  const hosts = useKnownHosts();
  const { values } = useFormState();

  const selectedHost = hosts.status === LoadingState.READY ? hosts.value?.selectedHost : undefined;

  const [useStoredPasswordLabel, setUseStoredPasswordLabel] = useState(false);
  const [storedHashInvalidated, setStoredHashInvalidated] = useState(false);

  const canUseStoredPassword = (remember: boolean, password: string | undefined) =>
    Boolean(remember && selectedHost?.hashedPassword && !password && !storedHashInvalidated);

  const togglePasswordLabel = (on: boolean) => setUseStoredPasswordLabel(on);

  // Host-sync: when the selected host changes, mirror its username + stored-
  // password hint into the form. Deliberately does NOT touch autoConnect — the
  // persisted setting is decoupled from which host is currently picked.
  useEffect(() => {
    if (!selectedHost) {
      return;
    }

    form.change('userName', selectedHost.userName);
    form.change('password', '');
    form.change('remember', Boolean(selectedHost.remember));

    setStoredHashInvalidated(false);
    togglePasswordLabel(
      Boolean(selectedHost.remember && selectedHost.hashedPassword)
    );
  }, [selectedHost, form]);

  // Mirror the persisted autoConnect setting into the form checkbox so the
  // field reflects truth as soon as settings load.
  useEffect(() => {
    if (settings.status !== LoadingState.READY) {
      return;
    }
    form.change('autoConnect', settings.value?.autoConnect);
  }, [settings, form]);

  const onUserNameChange = (userName: string | undefined) => {
    const fieldChanged = selectedHost?.userName?.toLowerCase() !== userName?.toLowerCase();
    if (canUseStoredPassword(values.remember, values.password) && fieldChanged) {
      setStoredHashInvalidated(true);
    }
  };

  const onRememberChange = (checked: boolean) => {
    // When the user unchecks "remember password", the auto-connect checkbox
    // can't meaningfully stay on (there are no saved credentials to use), so
    // reflect that in the form UI. Note: this writes only to the form field,
    // NOT to the persisted setting — toggling host-level remember is not a
    // user intent to change the app-level auto-connect preference.
    if (!checked && values.autoConnect) {
      form.change('autoConnect', false);
    }

    togglePasswordLabel(canUseStoredPassword(checked, values.password));
  };

  // User-initiated toggle of the auto-connect checkbox. This is the ONLY path
  // that writes to the persisted setting — wired directly to the Checkbox's
  // native onChange (see JSX below), not to a <OnChange> listener, because
  // OnChange fires on programmatic form.change calls too (host-sync effects
  // etc.) and would leak those into Dexie.
  const onUserToggleAutoConnect = (checked: boolean, fieldOnChange: (v: boolean) => void) => {
    fieldOnChange(checked);

    if (settings.status === LoadingState.READY) {
      void settings.update({ autoConnect: checked });
    }

    if (checked && !values.remember) {
      form.change('remember', true);
    }
  };

  return (
    <form className="loginForm" onSubmit={handleSubmit}>
      <div className="loginForm-items">
        <div className="loginForm-item">
          <Field
            label={t('Common.label.username')}
            name="userName"
            component={InputField}
            autoComplete="username"
          />
          <OnChange name="userName">{onUserNameChange}</OnChange>
        </div>
        <div className="loginForm-item">
          <Field
            label={useStoredPasswordLabel ? STORED_PASSWORD_LABEL : PASSWORD_LABEL}
            onFocus={() => setUseStoredPasswordLabel(false)}
            onBlur={() =>
              togglePasswordLabel(canUseStoredPassword(values.remember, values.password))
            }
            name="password"
            type="password"
            component={InputField}
            autoComplete="new-password"
          />
        </div>
        <div className="loginForm-actions">
          <Field
            label={t('LoginForm.label.savePassword')}
            name="remember"
            component={CheckboxField}
          />
          <OnChange name="remember">{onRememberChange}</OnChange>

          <Button color="primary" onClick={onResetPassword}>
            {t('LoginForm.label.forgot')}
          </Button>
        </div>
        <div className="loginForm-item">
          <Field name="selectedHost" component={KnownHosts} />
        </div>
        <div className="loginForm-actions">
          <Field name="autoConnect" type="checkbox">
            {({ input }) => (
              <FormControlLabel
                className="checkbox-field"
                label={t('LoginForm.label.autoConnect')}
                control={
                  <Checkbox
                    className="checkbox-field__box"
                    checked={!!input.value}
                    onChange={(_e, checked) => onUserToggleAutoConnect(checked, input.onChange)}
                    color="primary"
                  />
                }
              />
            )}
          </Field>
        </div>
      </div>
      <Button
        className="loginForm-submit rounded tall"
        color="primary"
        variant="contained"
        type="submit"
        disabled={disableSubmitButton}
      >
        {t('LoginForm.label.login')}
      </Button>
    </form>
  );
};

const LoginForm = (props: LoginFormProps) => {
  const { t } = useTranslation();

  const validate = (values: any) => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  };

  const handleOnSubmit = ({ userName, ...values }: any) => {
    userName = userName?.trim();
    props.onSubmit({ userName, ...values });
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => (
        <LoginFormBody {...props} form={form} handleSubmit={handleSubmit} />
      )}
    </Form>
  );
};

export default LoginForm;
