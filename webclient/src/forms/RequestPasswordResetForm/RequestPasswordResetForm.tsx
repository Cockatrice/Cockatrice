// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from 'react-redux';
import { Form, Field } from 'react-final-form';
import { OnChange } from 'react-final-form-listeners';

import Button from '@material-ui/core/Button';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './RequestPasswordResetForm.css';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';

const RequestPasswordResetForm = (props) => {
  const { onSubmit } = props;
  const [errorMessage, setErrorMessage] = useState(false);
  const [isMFA, setIsMFA] = useState(false);

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  useReduxEffect(() => {
    setIsMFA(true);
  }, ServerTypes.RESET_PASSWORD_CHALLENGE, []);

  const handleOnSubmit = (form) => {
    setErrorMessage(false);
    onSubmit(form);
  }

  const validate = values => {
    const errors: any = {};

    if (!values.userName) {
      errors.userName = 'Required';
    }
    if (isMFA && !values.email) {
      errors.email = 'Required';
    }
    if (!values.selectedHost) {
      errors.selectedHost = 'Required';
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
              {errorMessage ? (
                <div className="RequestPasswordResetForm-Error">Request Password Reset Failed, please try again</div>
              ) : null}
              <div className="RequestPasswordResetForm-item">
                <Field label="Username" name="userName" component={InputField} autoComplete="username" disabled={isMFA} />
              </div>
              {isMFA ? (
                <div className="RequestPasswordResetForm-item">
                  <Field label="Email" name="email" component={InputField} autoComplete="email" />
                  <div>Server has multi-factor authentication enabled</div>
                </div>
              ) : null}
              <div className="RequestPasswordResetForm-item selectedHost">
                <Field name='selectedHost' component={KnownHosts} disabled={isMFA} />
                <OnChange name="selectedHost">{onHostChange}</OnChange>
              </div>
            </div>
            <Button className="RequestPasswordResetForm-submit rounded tall" color="primary" variant="contained" type="submit">
              Request Reset Token
            </Button>
          </form>
        );
      }}
    </Form>
  );
};

export default RequestPasswordResetForm;
