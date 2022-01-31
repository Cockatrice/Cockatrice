import React, { useEffect, useState, useCallback } from 'react';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';

import Button from '@material-ui/core/Button';

import { AuthenticationService } from 'api';
import { CheckboxField, InputField, KnownHosts } from 'components';
import { useAutoConnect } from 'hooks';
import { HostDTO, SettingDTO } from 'services';
import { APP_USER } from 'types';

import './LoginForm.css';

const PASSWORD_LABEL = 'Password';
const STORED_PASSWORD_LABEL = '* SAVED *';

const LoginForm = ({ onSubmit, disableSubmitButton, onResetPassword }: LoginFormProps) => {
  const [host, setHost] = useState(null);
  const [passwordLabel, setPasswordLabel] = useState(PASSWORD_LABEL);
  const [autoConnect, setAutoConnect] = useAutoConnect();

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = 'Required';
    }
    if (!values.password && !values.selectedHost?.hashedPassword) {
      errors.password = 'Required';
    }
    if (!values.selectedHost) {
      errors.selectedHost = 'Required';
    }

    return errors;
  }

  const useStoredPassword = (remember) => remember && host.hashedPassword;
  const togglePasswordLabel = (useStoredLabel) => {
    setPasswordLabel(useStoredLabel ? STORED_PASSWORD_LABEL : PASSWORD_LABEL);
  };

  return (
    <Form onSubmit={onSubmit} validate={validate}>
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
          togglePasswordLabel(useStoredPassword(host.remember));
        }, [host]);

        const onUserNameChange = (userName) => {
          const fieldChanged = host.userName?.toLowerCase() !== values.userName?.toLowerCase();
          if (useStoredPassword(values.remember) && fieldChanged) {
            setHost(({ hashedPassword, ...s }) => ({ ...s, userName }));
          }
        }

        const onRememberChange = (checked) => {
          form.change('remember', checked);

          if (!checked && values.autoConnect) {
            onAutoConnectChange(false);
          }

          togglePasswordLabel(useStoredPassword(checked));
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
                <Field label='Username' name='userName' component={InputField} autoComplete='off' />
                <OnChange name="userName">{onUserNameChange}</OnChange>
              </div>
              <div className='loginForm-item'>
                <Field
                  label={passwordLabel}
                  onFocus={() => setPasswordLabel(PASSWORD_LABEL)}
                  onBlur={() => togglePasswordLabel(useStoredPassword(values.remember))}
                  name='password'
                  type='password'
                  component={InputField}
                  autoComplete='new-password'
                />
              </div>
              <div className='loginForm-actions'>
                <Field label='Save Password' name='remember' component={CheckboxField} />
                <OnChange name="remember">{onRememberChange}</OnChange>

                <Button color='primary' onClick={onResetPassword}>Forgot Password</Button>
              </div>
              <div className='loginForm-item'>
                <Field name='selectedHost' component={KnownHosts} />
                <OnChange name="selectedHost">{setHost}</OnChange>
              </div>
              <div className='loginForm-actions'>
                <Field label='Auto Connect' name='autoConnect' component={CheckboxField} />
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
              Login
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
