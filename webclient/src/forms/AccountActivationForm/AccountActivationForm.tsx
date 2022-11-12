// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './AccountActivationForm.css';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';

const AccountActivationForm = ({ onSubmit }) => {
  const [errorMessage, setErrorMessage] = useState(false);
  const { t } = useTranslation();

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.ACCOUNT_ACTIVATION_FAILED, []);

  const handleOnSubmit = ({ token, ...values }) => {
    setErrorMessage(false);

    token = token?.trim();

    onSubmit({ token, ...values });
  }

  const validate = values => {
    const errors: any = {};

    if (!values.token) {
      errors.token = t('Common.validation.required');
    }

    return errors;
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => {
        return (
          <form className="AccountActivationForm" onSubmit={handleSubmit}>
            <div className="AccountActivationForm-item">
              <Field label={t('Common.label.token')} name="token" component={InputField} />
            </div>

            {errorMessage && (
              <div className="AccountActivationForm-error">
                <Typography color="error">{ t('AccountActivationForm.error.failed') }</Typography>
              </div>
            )}

            <Button className="AccountActivationForm-submit rounded tall" color="primary" variant="contained" type="submit">
              { t('AccountActivationForm.label.activate') }
            </Button>
          </form>
        );
      }}
    </Form>
  );
};

export default AccountActivationForm;
