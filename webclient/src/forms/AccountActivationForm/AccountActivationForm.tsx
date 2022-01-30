// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';

import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './AccountActivationForm.css';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';

const AccountActivationForm = ({ onSubmit }) => {
  const [errorMessage, setErrorMessage] = useState(false);

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.ACCOUNT_ACTIVATION_FAILED, []);

  const handleOnSubmit = (form) => {
    setErrorMessage(false);
    onSubmit(form);
  }

  const validate = values => {
    const errors: any = {};

    if (!values.token) {
      errors.token = 'Required';
    }

    return errors;
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => {
        return (
          <form className="AccountActivationForm" onSubmit={handleSubmit}>
            <div className="AccountActivationForm-item">
              <Field label="Token" name="token" component={InputField} autoComplete="off" />
            </div>

            {errorMessage && (
              <div className="AccountActivationForm-error">
                <Typography color="error">Account activation failed</Typography>
              </div>
            )}

            <Button className="AccountActivationForm-submit rounded tall" color="primary" variant="contained" type="submit">
              Activate Account
            </Button>
          </form>
        );
      }}
    </Form>
  );
};

export default AccountActivationForm;
