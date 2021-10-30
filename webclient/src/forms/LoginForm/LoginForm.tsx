// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, Field, reduxForm, change } from "redux-form"

import Button from "@material-ui/core/Button";

import { InputField, KnownHosts } from "components";
// import { ServerDispatch } from "store";
import { FormKey } from 'types';

import "./LoginForm.css";

const LoginForm = (props) => {
  const { dispatch, handleSubmit } = props;

  const forgotPassword = () => {
    console.log('LoginForm.forgotPassword->openForgotPasswordDialog');
  };

  const onHostChange = ({ host, port }) => {
    dispatch(change(FormKey.LOGIN, 'host', host));
    dispatch(change(FormKey.LOGIN, 'port', port));
  }

  return (
    <Form className="loginForm" onSubmit={handleSubmit}>
      <div className="loginForm-items">
        <div className="loginForm-item">
          <Field label="User" name="user" component={InputField} autoComplete="username" />
        </div>
        <div className="loginForm-item">
          <Field label="Pass" name="pass" type="password" component={InputField} autoComplete="current-password" />
        </div>
        <div className="loginForm-actions">
          <span>Auto Connect</span>
          <Button color="primary" onClick={forgotPassword}>Forgot Password</Button>
        </div>
        <div className="loginForm-item">
          <KnownHosts onChange={onHostChange} />
        </div>
      </div>
      <Button className="loginForm-submit rounded tall" color="primary" variant="contained" type="submit">
        Login
      </Button>
    </Form>
  );
}

const propsMap = {
  form: FormKey.LOGIN,
  validate: values => {
    const errors: any = {};

    if (!values.user) errors.user = 'Required';
    if (!values.pass) errors.pass = 'Required';
    if (!values.host) errors.host = 'Required';
    if (!values.port) errors.port = 'Required';

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

export default connect(mapStateToProps)(reduxForm(propsMap)(LoginForm));
