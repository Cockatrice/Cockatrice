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

import './RequestPasswordResetForm.css';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';

const RequestPasswordResetForm = ({ onSubmit, skipTokenRequest }) => {
  const [errorMessage, setErrorMessage] = useState(false);
  const [isMFA, setIsMFA] = useState(false);
  const { t } = useTranslation();

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  useReduxEffect(() => {
    setIsMFA(true);
  }, ServerTypes.RESET_PASSWORD_CHALLENGE, []);

  const handleOnSubmit = ({ userName, email, ...values }) => {
    setErrorMessage(false);

    userName = userName?.trim();
    email = email?.trim();

    onSubmit({ userName, email, ...values });
  }

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = t('Common.validation.required');
    }
    if (isMFA && !values.email) {
      errors.email = t('Common.validation.required');
    }
    if (!values.selectedHost) {
      errors.selectedHost = t('Common.validation.required');
    }

    return errors;
  };

  return (
    <Form onSubmit={handleOnSubmit} validate={validate}>
      {({ handleSubmit, form }) => {
        const onHostChange: any = ({ userName }) => {
          form.change('userName', userName);
          setIsMFA(false);
        }

        return (
          <form className="RequestPasswordResetForm" onSubmit={handleSubmit}>
            <div className="RequestPasswordResetForm-items">
              <div className="RequestPasswordResetForm-item">
                <Field label={t('Common.label.username')} name="userName" component={InputField} autoComplete="username" disabled={isMFA} />
              </div>
              {isMFA ? (
                <div className="RequestPasswordResetForm-item">
                  <Field label={t('Common.label.email')} name="email" type="email" component={InputField} autoComplete="email" />
                  <div>{ t('RequestPasswordResetForm.mfaEnabled') }</div>
                </div>
              ) : null}
              <div className="RequestPasswordResetForm-item selectedHost">
                <Field name='selectedHost' component={KnownHosts} disabled={isMFA} />
                <OnChange name="selectedHost">{onHostChange}</OnChange>
              </div>

              {errorMessage && (
                <div className="RequestPasswordResetForm-item">
                  <Typography color="error">{ t('RequestPasswordResetForm.error') }</Typography>
                </div>
              )}
            </div>

            <Button className="RequestPasswordResetForm-submit rounded tall" color="primary" variant="contained" type="submit">
              { t('RequestPasswordResetForm.request') }
            </Button>

            <div>
              <Button color="primary" onClick={() => skipTokenRequest(form.getState().values.userName)}>
                { t('RequestPasswordResetForm.skipRequest') }
              </Button>
            </div>
          </form>
        );
      }}
    </Form>
  );
};

export default RequestPasswordResetForm;
