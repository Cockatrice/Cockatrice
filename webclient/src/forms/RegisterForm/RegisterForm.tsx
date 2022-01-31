// eslint-disable-next-line
import React, { Component, useState } from 'react';
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import setFieldTouched from 'final-form-set-field-touched'

import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';

import { CountryDropdown, InputField, KnownHosts } from 'components';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';
import { FormKey } from 'types';

import './RegisterForm.css';

const RegisterForm = ({ onSubmit }: RegisterFormProps) => {
  const [emailRequired, setEmailRequired] = useState(false);
  const [error, setError] = useState(null);
  const [emailError, setEmailError] = useState(null);
  const [passwordError, setPasswordError] = useState(null);
  const [userNameError, setUserNameError] = useState(null);

  const onHostChange = (host) => setEmailRequired(false);
  const onEmailChange = () => emailError && setEmailError(null);
  const onPasswordChange = () => passwordError && setPasswordError(null);
  const onUserNameChange = () => userNameError && setUserNameError(null);

  useReduxEffect(() => {
    setEmailRequired(true);
  }, ServerTypes.REGISTRATION_REQUIRES_EMAIL);

  useReduxEffect(({ error }) => {
    setError(error);
  }, ServerTypes.REGISTRATION_FAILED);

  useReduxEffect(({ error }) => {
    setEmailError(error);
  }, ServerTypes.REGISTRATION_EMAIL_ERROR);

  useReduxEffect(({ error }) => {
    setPasswordError(error);
  }, ServerTypes.REGISTRATION_PASSWORD_ERROR);

  useReduxEffect(({ error }) => {
    setUserNameError(error);
  }, ServerTypes.REGISTRATION_USERNAME_ERROR);

  const handleOnSubmit = form => {
    setError(null);
    onSubmit(form);
  }

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = 'Required';
    } else if (userNameError) {
      errors.userName = userNameError;
    }

    if (!values.password) {
      errors.password = 'Required';
    } else if (values.password.length < 8) {
      errors.password = 'Minimum of 8 characters required';
    } else if (passwordError) {
      errors.password = passwordError;
    }

    if (!values.passwordConfirm) {
      errors.passwordConfirm = 'Required';
    } else if (values.password !== values.passwordConfirm) {
      errors.passwordConfirm = 'Passwords don\'t match'
    }

    if (!values.selectedHost) {
      errors.selectedHost = 'Required';
    }

    if (emailRequired && !values.email) {
      errors.email = 'Required';
    } else if (emailError) {
      errors.email = emailError;
    }

    return errors;
  }

  return (
    <Form onSubmit={handleOnSubmit} validate={validate} mutators={{ setFieldTouched }}>
      {({ handleSubmit, form, ...args }) => {
        const { values } = form.getState();

        if (emailRequired) {
          // Allow form render to complete
          setTimeout(() => form.mutators.setFieldTouched('email', true))
        }

        return (
          <>
            <form className="RegisterForm" onSubmit={handleSubmit} autoComplete="off">
              <div className="RegisterForm-column">
                <div className="RegisterForm-item">
                  <Field label="Player Name" name="userName" component={InputField} />
                  <OnChange name="userName">{onUserNameChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field label="Password" name="password" type="password" component={InputField} autoComplete='new-password' />
                  <OnChange name="password">{onPasswordChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field
                    label="Confirm Password"
                    name="passwordConfirm"
                    type="password"
                    component={InputField}
                    autoComplete='new-password'
                  />
                </div>
                <div className="RegisterForm-item">
                  <Field name="selectedHost" component={KnownHosts} />
                  <OnChange name="selectedHost">{onHostChange}</OnChange>
                </div>
              </div>
              <div className="RegisterForm-column" >
                <div className="RegisterForm-item">
                  <Field label="Real Name" name="realName" component={InputField} autoComplete='off' />
                </div>
                <div className="RegisterForm-item">
                  <Field label="Email" name="email" type="email" component={InputField} />
                  <OnChange name="email">{onEmailChange}</OnChange>
                </div>
                <div className="RegisterForm-item">
                  <Field label="Country" name="country" component={CountryDropdown} />
                </div>
                <Button className="RegisterForm-submit tall" color="primary" variant="contained" type="submit">
                  Register
                </Button>
              </div>
            </form>

            { error && (
              <div className="RegisterForm-item">
                <Typography color="error">{error}</Typography>
              </div>
            )}
          </>
        );
      }}

    </Form >
  );
};

interface RegisterFormProps {
  onSubmit: any;
}

export default RegisterForm;
