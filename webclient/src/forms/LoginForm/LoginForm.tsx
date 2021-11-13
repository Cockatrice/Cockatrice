// eslint-disable-next-line
import React, { Component, useCallback, useEffect, useState } from 'react';
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change, FormSubmitHandler } from 'redux-form'

import Button from '@material-ui/core/Button';

import { AuthenticationService } from 'api';
import { CheckboxField, InputField, KnownHosts } from 'components';
import { HostDTO, SettingDTO } from 'services';
import { FormKey, APP_USER } from 'types';

import './LoginForm.css';

const LoginForm: any = ({ dispatch, form, submit, handleSubmit }: LoginFormProps) => {
  const [autoConnect, setAutoConnect] = useState(false);

  useEffect(() => {
    SettingDTO.get(APP_USER).then((userSetting: SettingDTO) => {
      if (!userSetting) {
        new SettingDTO(APP_USER).save();
        return;
      }

      if (userSetting.autoConnect) {
        setAutoConnect(userSetting.autoConnect);

        if (!AuthenticationService.connectionAttemptMade()) {
          HostDTO.getAll().then(hosts => {
            let lastSelectedHost = hosts.find(({ lastSelected }) => lastSelected);

            if (lastSelectedHost?.remember) {
              dispatch(change(form, 'selectedHost', lastSelectedHost));
              dispatch(change(form, 'user', lastSelectedHost.user));
              dispatch(change(form, 'pass', lastSelectedHost.pass));
              dispatch(change(form, 'remember', lastSelectedHost.remember));
              dispatch(submit);
            }
          });
        }
      }
    });
  }, [dispatch, form, submit]);

  useEffect(() => {
    dispatch(change(form, 'autoConnect', autoConnect));

    SettingDTO.get(APP_USER).then((setting: SettingDTO) => {
      setting.autoConnect = autoConnect;
      setting.save();
    });
  }, [dispatch, form, autoConnect]);

  const forgotPassword = () => {
    console.log('Show recover password dialog, then AuthService.forgotPasswordRequest');
  };

  const onRememberMeChange = event => {
    if (autoConnect && !event.target.checked) {
      setAutoConnect(false);
    }
  }

  const onAutoConnectChange = event => {
    setAutoConnect(event.target.checked);

    if (event.target.checked) {
      dispatch(change(form, 'remember', true));
    }
  }

  const onHostChange = host => {
    dispatch(change(form, 'user', host.user));
    dispatch(change(form, 'pass', host.pass));
    dispatch(change(form, 'remember', host.remember));
    setAutoConnect(host.remember && autoConnect);
  }

  return (
    <Form className='loginForm' onSubmit={handleSubmit}>
      <div className='loginForm-items'>
        <div className='loginForm-item'>
          <Field label='Username' name='user' component={InputField} autoComplete='off' />
        </div>
        <div className='loginForm-item'>
          <Field label='Password' name='pass' type='password' component={InputField} autoComplete='new-password' />
        </div>
        <div className='loginForm-actions'>
          {/* @TODO: change label to Save Password when passHash is supported */}
          <Field label='Save Password' name='remember' component={CheckboxField} onChange={onRememberMeChange} />
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
    if (!values.pass) {
      errors.pass = 'Required';
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
