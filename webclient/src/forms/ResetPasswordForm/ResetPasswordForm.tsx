// eslint-disable-next-line
import React, { useEffect, useState } from 'react';
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form'
import { OnChange } from 'react-final-form-listeners'

import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './ResetPasswordForm.css';
import { useReduxEffect } from '../../hooks';
import { ServerTypes } from '../../store';

const ResetPasswordForm = ({ onSubmit, userName }) => {
  const [errorMessage, setErrorMessage] = useState(false);

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = 'Required';
    }
    if (!values.token) {
      errors.token = 'Required';
    }

    if (!values.newPassword) {
      errors.newPassword = 'Required';
    } else if (values.newPassword.length < 8) {
      errors.password = 'Minimum of 8 characters required';
    }

    if (!values.passwordAgain) {
      errors.passwordAgain = 'Required';
    } else if (values.newPassword !== values.passwordAgain) {
      errors.passwordAgain = 'Passwords don\'t match'
    }
    if (!values.selectedHost) {
      errors.selectedHost = 'Required';
    }

    return errors;
  };

  return (
    <Form onSubmit={onSubmit} validate={validate} initialValues={{ userName }}>
      {({ handleSubmit, form }) => (
        <form className='ResetPasswordForm' onSubmit={handleSubmit}>
          <div className='ResetPasswordForm-items'>
            <div className='ResetPasswordForm-item'>
              <Field label='Username' name='userName' component={InputField} autoComplete='username' disabled={!!userName} />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field label='Token' name='token' component={InputField} />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field label='Password' name='newPassword' type='password' component={InputField} autoComplete='new-password' />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field label='Password Again' name='passwordAgain' type='password' component={InputField} autoComplete='new-password' />
            </div>
            <div className='ResetPasswordForm-item'>
              <Field name='selectedHost' component={KnownHosts} disabled />
            </div>

            {errorMessage && (
              <div className='ResetPasswordForm-item'>
                <Typography color="error">Password reset failed</Typography>
              </div>
            )}
          </div>
          <Button className='ResetPasswordForm-submit rounded tall' color='primary' variant='contained' type='submit'>
            Reset Password
          </Button>
        </form>
      )}
    </Form>
  );
};

export default ResetPasswordForm;
