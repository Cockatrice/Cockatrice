// eslint-disable-next-line
import React, { useState } from "react";
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change } from 'redux-form'

import Button from '@material-ui/core/Button';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './RequestPasswordResetForm.css';
import { useReduxEffect } from 'hooks';
import { ServerTypes } from 'store';

const RequestPasswordResetForm = (props) => {
  const { dispatch, handleSubmit } = props;
  const [errorMessage, setErrorMessage] = useState(false);
  const [isMFA, setIsMFA] = useState(false);

  const onHostChange: any = ({ host, port }) => {
    dispatch(change(FormKey.RESET_PASSWORD_REQUEST, 'host', host));
    dispatch(change(FormKey.RESET_PASSWORD_REQUEST, 'port', port));
  }

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);

  useReduxEffect(() => {
    setIsMFA(true);
  }, ServerTypes.RESET_PASSWORD_CHALLENGE, []);

  const onSubmit = (event) => {
    setErrorMessage(false);
    handleSubmit(event);
  }

  return (
    <Form className="RequestPasswordResetForm" onSubmit={onSubmit}>
      <div className="RequestPasswordResetForm-items">
        {errorMessage ? (
          <div className="RequestPasswordResetForm-Error">Request Password Reset Failed, please try again</div>
        ) : null}
        <div className="RequestPasswordResetForm-item">
          <Field label="Username" name="user" component={InputField} autoComplete="username" />
        </div>
        {isMFA ? (
          <div className="RequestPasswordResetForm-item">
            <div className="RequestPasswordResetForm-MFA-Message">Server has multi-factor authentication enabled</div>
            <Field label="Email" name="email" component={InputField} autoComplete="email" />
          </div>
        ) : null}
        <div className="RequestPasswordResetForm-item">
          <Field name='selectedHost' component={KnownHosts} onChange={onHostChange} />
        </div>
      </div>
      <Button className="RequestPasswordResetForm-submit rounded tall" color="primary" variant="contained" type="submit">
        Request Reset Token
      </Button>
    </Form>
  );
};

const propsMap = {
  form: FormKey.RESET_PASSWORD_REQUEST,
  validate: values => {
    const errors: any = {};

    if (!values.user) {
      errors.user = 'Required';
    }
    if (!values.host) {
      errors.host = 'Required';
    }
    if (!values.port) {
      errors.port = 'Required';
    }

    return errors;
  }
};

const mapStateToProps = () => ({
  initialValues: {
    // host: "mtg.tetrarch.co/servatrice",
    // port: "443"
    // host: "server.cockatrice.us",
    // port: "4748"
  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(RequestPasswordResetForm));
