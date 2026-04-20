import React from 'react';
import { Form, Field, FormApi } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Checkbox from '@mui/material/Checkbox';
import CircularProgress from '@mui/material/CircularProgress';
import FormControlLabel from '@mui/material/FormControlLabel';

import { CheckboxField, InputField, KnownHosts } from '@app/components';
import type { FormErrors } from '@app/forms';
import { LoadingState, useKnownHosts, useSettings } from '@app/hooks';
import { HostDTO } from '@app/services';

import { useLoginFormBody } from './useLoginForm';

import './LoginForm.css';

export interface LoginFormValues {
  userName: string;
  password: string;
  remember: boolean;
  autoConnect: boolean;
  selectedHost: HostDTO;
}

interface LoginFormProps {
  onSubmit: (values: LoginFormValues) => void;
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
  const STORED_PASSWORD_LABEL = t('LoginForm.label.savedPassword');

  const {
    useStoredPasswordLabel,
    setUseStoredPasswordLabel,
    onSelectedHostChange,
    onUserNameChange,
    onRememberChange,
    onUserToggleAutoConnect,
    passwordFieldBlur,
  } = useLoginFormBody(form);

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
            onBlur={passwordFieldBlur}
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
          <OnChange name="selectedHost">{onSelectedHostChange}</OnChange>
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
  const knownHosts = useKnownHosts();
  const settings = useSettings();

  const validate = (values: Partial<LoginFormValues>): FormErrors<LoginFormValues> => {
    const errors: FormErrors<LoginFormValues> = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  };

  const handleOnSubmit = ({ userName, ...values }: LoginFormValues) => {
    props.onSubmit({ ...values, userName: userName?.trim() });
  };

  if (knownHosts.status !== LoadingState.READY || settings.status !== LoadingState.READY) {
    return (
      <div className="loginForm-loading">
        <CircularProgress size={40} />
      </div>
    );
  }

  const selectedHost = knownHosts.value?.selectedHost;
  const initialValues: Partial<LoginFormValues> = {
    selectedHost,
    userName: selectedHost?.userName ?? '',
    remember: Boolean(selectedHost?.remember),
    autoConnect: Boolean(settings.value?.autoConnect),
    password: '',
  };

  return (
    <Form
      onSubmit={handleOnSubmit}
      validate={validate}
      initialValues={initialValues}
      keepDirtyOnReinitialize
    >
      {({ handleSubmit, form }) => (
        <LoginFormBody {...props} form={form} handleSubmit={handleSubmit} />
      )}
    </Form>
  );
};

export default LoginForm;
