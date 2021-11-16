// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Form, Field, reduxForm, change } from 'redux-form'

import Button from '@material-ui/core/Button';

import { InputField, KnownHosts } from 'components';
import { FormKey } from 'types';

import './RegisterForm.css';

const RegisterForm = (props) => {
  const { dispatch, handleSubmit } = props;

  const onHostChange = ({ host, port }) => {
    dispatch(change(FormKey.REGISTER, 'host', host));
    dispatch(change(FormKey.REGISTER, 'port', port));
  }
  return (
    <Form className="registerForm row" onSubmit={handleSubmit} autoComplete="off">
      <div className="leftRegisterForm column" >
        <div className="registerForm-item">
          <KnownHosts onChange={onHostChange} />
          { /* Padding is off */ }
        </div>
        <div className="registerForm-item">
          <Field label="Country" name="country" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Real Name" name="realName" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Email" name="email" type="email" component={InputField} />
        </div>
      </div>
      <div className="rightRegisterForm column">
        <div className="registerForm-item">
          <Field label="Player Name" name="user" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Password" name="pass" type="password" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Password (again)" name="passwordConfirm" type="password" component={InputField} />
        </div>
        <Button className="registerForm-submit tall" color="primary" variant="contained" type="submit">
          Register
        </Button>
      </div>

    </Form >
  );
};

const propsMap = {
  form: FormKey.REGISTER,
};

const mapStateToProps = () => ({
  initialValues: {

  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(RegisterForm));
