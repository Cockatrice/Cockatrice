// eslint-disable-next-line
import React, { useState } from "react";
import { Form, Field } from 'react-final-form';
import { useTranslation } from 'react-i18next';

import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';

import { InputField } from '@app/components';
import { useReduxEffect } from '@app/hooks';
import { ServerTypes } from '@app/store';

import './AccountActivationForm.css';

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
      {({ handleSubmit }) => {
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
