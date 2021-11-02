// eslint-disable-next-line
import React, { Component, useCallback, useEffect, useState } from 'react';
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change, FormSubmitHandler } from 'redux-form'

import Button from '@material-ui/core/Button';

import { CheckboxField, InputField, KnownHosts } from 'components';
import { SettingDTO } from 'services';
import { ServerSelectors } from 'store';
import { FormKey, APP_USER } from 'types';

import './LoginForm.css';


const LoginForm: any = ({ dispatch, form, submit, connectionDescription, handleSubmit }: LoginFormProps) => {
  // This isnt getting set right
  const [autoConnect, setAutoConnect] = useState(false);

  useEffect(() => {
    (async() => {
      const userSetting: SettingDTO = await SettingDTO.get(APP_USER);

      setTimeout(() => {
        setAutoConnect(userSetting?.autoConnect);

        if (!userSetting) {
          new SettingDTO(APP_USER).save();
        } else if (userSetting.autoConnect) {
          dispatch(change(form, 'autoConnect', true));

          if (!connectionDescription) {
            dispatch(submit);
          }
        }
      }, 100);
    })();
  // @TODO: figure out missing dependencies in a way that doesnt have this fire more than intended
  }, []);

  const forgotPassword = () => {
    console.log('Show recover password dialog, then AuthService.forgotPasswordRequest');
  };

  const onAutoConnectChange = event => {
    if (event.target.checked) {
      dispatch(change(form, 'remember', true));
    }
  }

  const onRememberMeChange = event => {
    if (!event.target.checked) {
      dispatch(change(form, 'autoConnect', false));
    }
  }

  const onHostChange = async host => {
    console.log('onHostChange');
    dispatch(change(form, 'user', host.user));
    dispatch(change(form, 'pass', host.pass));
    // this isnt getting set right
    dispatch(change(form, 'remember', host.remember || autoConnect));
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
  connectionDescription: number;
  form: string;
  dispatch: Function;
  submit: Function;
  handleSubmit: FormSubmitHandler;
}

const mapStateToProps = (state) => ({
  connectionDescription: ServerSelectors.getDescription(state),
});

export default connect(mapStateToProps)(reduxForm(propsMap)(LoginForm));
