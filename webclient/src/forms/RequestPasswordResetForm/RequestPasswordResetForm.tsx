// eslint-disable-next-line
import React from "react";
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change } from 'redux-form'

import Button from '@material-ui/core/Button';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './RequestPasswordResetForm.css';

const RequestPasswordResetForm = (props) => {
  const { dispatch, handleSubmit } = props;

  const onHostChange = ({ host, port }) => {
    dispatch(change(FormKey.RESET_PASSWORD_REQUEST, 'host', host));
    dispatch(change(FormKey.RESET_PASSWORD_REQUEST, 'port', port));
  }

  return (
    <Form className="RequestPasswordResetForm" onSubmit={handleSubmit}>
      <div className="RequestPasswordResetForm-items">
        <div className="RequestPasswordResetForm-item">
          <Field label="Username" name="user" component={InputField} autoComplete="username" />
        </div>
        <div className="RequestPasswordResetForm-item">
          <KnownHosts onChange={onHostChange} />
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
