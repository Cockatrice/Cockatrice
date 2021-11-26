// eslint-disable-next-line
import React, {useState} from "react";
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change } from 'redux-form'

import Button from '@material-ui/core/Button';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './ResetPasswordForm.css';
import { useReduxEffect } from '../../hooks';
import { ServerTypes } from '../../store';

const ResetPasswordForm = (props) => {
  const { dispatch, handleSubmit } = props;

  const [errorMessage, setErrorMessage] = useState(false);


  const onHostChange: any = ({ host, port }) => {
    dispatch(change(FormKey.RESET_PASSWORD, 'host', host));
    dispatch(change(FormKey.RESET_PASSWORD, 'port', port));
  }

  useReduxEffect(() => {
    setErrorMessage(true);
  }, ServerTypes.RESET_PASSWORD_FAILED, []);


  return (
    <Form className="ResetPasswordForm" onSubmit={handleSubmit}>
      <div className="ResetPasswordForm-items">
        {errorMessage ? (
          <div><h3>Password Reset Failed, please try again</h3></div>
        ) : null}
        <div className="ResetPasswordForm-item">
          <Field label="Username" name="user" component={InputField} autoComplete="username" />
        </div>
        <div className="ResetPasswordForm-item">
          <Field label="Token" name="token" component={InputField} />
        </div>
        <div className="ResetPasswordForm-item">
          <Field label="Password" name="newPassword" component={InputField} />
        </div>
        <div className="ResetPasswordForm-item">
          <Field label="Password Again" name="passwordAgain" component={InputField} />
        </div>
        <div className="ResetPasswordForm-item">
          <Field name='selectedHost' component={KnownHosts} onChange={onHostChange} />
        </div>
      </div>
      <Button className="ResetPasswordForm-submit rounded tall" color="primary" variant="contained" type="submit">
        Change Password
      </Button>
    </Form>
  );
};

const propsMap = {
  form: FormKey.RESET_PASSWORD,
  validate: values => {
    const errors: any = {};

    if (!values.user) {
      errors.user = 'Required';
    }
    if (!values.token) {
      errors.token = 'Required';
    }
    if (!values.newPassword) {
      errors.newPassword = 'Required';
    }
    if (!values.passwordAgain) {
      errors.passwordAgain = 'Required';
    } else if (values.newPassword !== values.passwordAgain) {
      errors.passwordAgain = 'Passwords don\'t match'
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

export default connect(mapStateToProps)(reduxForm(propsMap)(ResetPasswordForm));
