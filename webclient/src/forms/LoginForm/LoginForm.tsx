// eslint-disable-next-line
import React, { Component, useCallback, useEffect, useState, useRef } from 'react';
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change, FormSubmitHandler } from 'redux-form'

import Button from '@material-ui/core/Button';

import { AuthenticationService } from 'api';
import { CheckboxField, InputField, KnownHosts } from 'components';
import { useAutoConnect } from 'hooks';
import { HostDTO, SettingDTO } from 'services';
import { FormKey, APP_USER } from 'types';

import './LoginForm.css';

const PASSWORD_LABEL = 'Password';
const STORED_PASSWORD_LABEL = '* SAVED *';

const LoginForm: any = ({ dispatch, form, submit, handleSubmit }: LoginFormProps) => {
  const password: any = useRef();
  const [host, setHost] = useState(null);
  const [remember, setRemember] = useState(false);
  const [passwordLabel, setPasswordLabel] = useState(PASSWORD_LABEL);
  const [hasStoredPassword, useStoredPassword] = useState(false);

  const [autoConnect, setAutoConnect] = useAutoConnect(() => {
    dispatch(change(form, 'autoConnect', autoConnect));

    if (autoConnect && !remember) {
      setRemember(true);
    }
  });

  useEffect(() => {
    SettingDTO.get(APP_USER).then((userSetting: SettingDTO) => {
      if (userSetting?.autoConnect && !AuthenticationService.connectionAttemptMade()) {
        HostDTO.getAll().then(hosts => {
          let lastSelectedHost = hosts.find(({ lastSelected }) => lastSelected);

          if (lastSelectedHost?.remember && lastSelectedHost?.hashedPassword) {
            dispatch(change(form, 'selectedHost', lastSelectedHost));
            dispatch(change(form, 'userName', lastSelectedHost.userName));
            dispatch(change(form, 'remember', true));
            setPasswordLabel(STORED_PASSWORD_LABEL);
            dispatch(submit);
          }
        });
      }
    });
  }, [submit, dispatch, form]);

  useEffect(() => {
    dispatch(change(form, 'remember', remember));

    if (!remember) {
      setAutoConnect(false);
    }

    if (!remember) {
      useStoredPassword(false);
      setPasswordLabel(PASSWORD_LABEL);
    } else if (host?.hashedPassword) {
      useStoredPassword(true);
      setPasswordLabel(STORED_PASSWORD_LABEL);
    }
  }, [remember, dispatch, form]);

  useEffect(() => {
    if (!host) {
      return
    }

    dispatch(change(form, 'userName', host.userName));
    dispatch(change(form, 'password', ''));

    setRemember(host.remember);
    setAutoConnect(host.remember && autoConnect);

    if (host.remember && host.hashedPassword) {
      // TODO: check if this causes a double render (maybe try combined state)
      // try deriving useStoredPassword
      useStoredPassword(true);
      setPasswordLabel(STORED_PASSWORD_LABEL);
    } else {
      useStoredPassword(false);
      setPasswordLabel(PASSWORD_LABEL);
    }
  }, [host, dispatch, form]);

  const onRememberChange = event => setRemember(event.target.checked);
  const onAutoConnectChange = event => setAutoConnect(event.target.checked);
  const onHostChange = h => setHost(h);

  const forgotPassword = () => {
    console.log('Show recover password dialog, then AuthService.forgotPasswordRequest');
  };

  return (
    <Form className='loginForm' onSubmit={handleSubmit}>
      <div className='loginForm-items'>
        <div className='loginForm-item'>
          <Field label='Username' name='userName' component={InputField} autoComplete='off' />
        </div>
        <div className='loginForm-item'>
          <Field
            label={passwordLabel}
            ref={password}
            onFocus={() => setPasswordLabel(PASSWORD_LABEL)}
            onBlur={() => !password.current.value && hasStoredPassword && setPasswordLabel(STORED_PASSWORD_LABEL)}
            name='password'
            type='password'
            component={InputField}
            autoComplete='new-password'
          />
        </div>
        <div className='loginForm-actions'>
          <Field label='Save Password' name='remember' component={CheckboxField} onChange={onRememberChange} />
          <Button color='primary' onClick={forgotPassword}>Forgot Password</Button>
        </div>
        <div className='loginForm-item'>
          <Field name='selectedHost' component={KnownHosts} onChange={onHostChange} />
        </div>
        <div className='loginForm-actions'>
          <Field label='Auto Connect' name='autoConnect' component={CheckboxField} onChange={onAutoConnectChange} />
        </div>
      </div>
      <Button className='loginForm-submit rounded tall' color='primary' variant='contained' type='submit'>
        Login
      </Button>
    </Form>
  );
};

const propsMap = {
  form: FormKey.LOGIN,
  validate: values => {
    const errors: any = {};

    if (!values.user) {
      errors.user = 'Required';
    }

    if (!values.password && !values.selectedHost?.hashedPassword) {
      errors.password = 'Required';
    }

    if (!values.selectedHost) {
      errors.selectedHost = 'Required';
    }

    return errors;
  }
};

interface LoginFormProps {
  form: string;
  dispatch: Function;
  submit: Function;
  handleSubmit: FormSubmitHandler;
}

const mapStateToProps = (state) => ({

});

export default connect(mapStateToProps)(reduxForm(propsMap)(LoginForm));
