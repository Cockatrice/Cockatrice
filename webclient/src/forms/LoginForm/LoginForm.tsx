import React, { useEffect, useState, useCallback } from 'react';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';

import { AuthenticationService } from 'api';
import { CheckboxField, InputField, KnownHosts } from 'components';
import { useAutoConnect } from 'hooks';
import { HostDTO, SettingDTO } from 'services';
import { APP_USER } from 'types';

import './LoginForm.css';

const LoginForm = ({ onSubmit, disableSubmitButton, onResetPassword }: LoginFormProps) => {
  const { t } = useTranslation();
  const PASSWORD_LABEL = t('Common.label.password');
  const STORED_PASSWORD_LABEL = `* ${t('LoginForm.label.savedPassword')} *`;

  const [host, setHost] = useState(null);
  const [useStoredPasswordLabel, setUseStoredPasswordLabel] = useState(false);
  const [autoConnect, setAutoConnect] = useAutoConnect();

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  }

  const useStoredPassword = (remember, password) => remember && host?.hashedPassword && !password;
  const togglePasswordLabel = (useStoredLabel) => {
    setUseStoredPasswordLabel(useStoredLabel);
  };

  const handleOnSubmit = ({ userName, ...values }) => {
    userName = userName?.trim();
    console.log(userName, values);

    onSubmit({ userName, ...values });
  }

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => {
        const { values } = form.getState();

        useEffect(() => {
          SettingDTO.get(APP_USER).then((userSetting: SettingDTO) => {
            if (userSetting?.autoConnect && !AuthenticationService.connectionAttemptMade()) {
              HostDTO.getAll().then(hosts => {
                let lastSelectedHost = hosts.find(({ lastSelected }) => lastSelected);

                if (lastSelectedHost?.remember && lastSelectedHost?.hashedPassword) {
                  togglePasswordLabel(true);

                  form.change('selectedHost', lastSelectedHost);
                  form.change('userName', lastSelectedHost.userName);
                  form.change('remember', true);
                  form.submit();
                }
              });
            }
          });
        }, []);

        useEffect(() => {
          if (!host) {
            return;
          }

          form.change('userName', host.userName);
          form.change('password', '');

          onRememberChange(host.remember);
          onAutoConnectChange(host.remember && autoConnect);
          togglePasswordLabel(useStoredPassword(host.remember, values.password));
        }, [host]);

        const onUserNameChange = (userName) => {
          const fieldChanged = host?.userName?.toLowerCase() !== values.userName?.toLowerCase();
          if (useStoredPassword(values.remember, values.password) && fieldChanged) {
            setHost(({ hashedPassword, ...s }) => ({ ...s, userName }));
          }
        }

        const onRememberChange = (checked) => {
          form.change('remember', checked);

          if (!checked && values.autoConnect) {
            onAutoConnectChange(false);
          }

          togglePasswordLabel(useStoredPassword(checked, values.password));
        }

        const onAutoConnectChange = (checked) => {
          setAutoConnect(checked);

          form.change('autoConnect', checked);

          if (checked && !values.remember) {
            form.change('remember', true);
          }
        }

        return (
          <form className='loginForm' onSubmit={handleSubmit}>
            <div className='loginForm-items'>
              <div className='loginForm-item'>
                <Field label={t('Common.label.username')} name='userName' component={InputField} autoComplete='username' />
                <OnChange name="userName">{onUserNameChange}</OnChange>
              </div>
              <div className='loginForm-item'>
                <Field
                  label={useStoredPasswordLabel ? STORED_PASSWORD_LABEL : PASSWORD_LABEL}
                  onFocus={() => setUseStoredPasswordLabel(false)}
                  onBlur={() => togglePasswordLabel(useStoredPassword(values.remember, values.password))}
                  name='password'
                  type='password'
                  component={InputField}
                  autoComplete='new-password'
                />
              </div>
              <div className='loginForm-actions'>
                <Field label={t('LoginForm.label.savePassword')} name='remember' component={CheckboxField} />
                <OnChange name="remember">{onRememberChange}</OnChange>

                <Button color='primary' onClick={onResetPassword}>
                  { t('LoginForm.label.forgot') }
                </Button>
              </div>
              <div className='loginForm-item'>
                <Field name='selectedHost' component={KnownHosts} />
                <OnChange name="selectedHost">{setHost}</OnChange>
              </div>
              <div className='loginForm-actions'>
                <Field label={t('LoginForm.label.autoConnect')} name='autoConnect' component={CheckboxField} />
                <OnChange name="autoConnect">{onAutoConnectChange}</OnChange>
              </div>
            </div>
            <Button
              className='loginForm-submit rounded tall'
              color='primary'
              variant='contained'
              type='submit'
              disabled={disableSubmitButton}
            >
              { t('LoginForm.label.login') }
            </Button>
          </form>
        )
      }}
    </Form>
  );
};

interface LoginFormProps {
  onSubmit: any;
  disableSubmitButton: boolean,
  onResetPassword: any;
}

export default LoginForm;
