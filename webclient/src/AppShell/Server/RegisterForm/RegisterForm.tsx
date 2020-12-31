// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Form, Field, InjectedFormProps, reduxForm } from 'redux-form'

import Button from '@material-ui/core/Button';

import InputField from 'AppShell/common/components/InputField/InputField';

import './RegisterForm.css';

class RegisterForm extends Component<InjectedFormProps> {
  render() {
    return (
      <Form className="registerForm" onSubmit={this.props.handleSubmit} autoComplete="off">
        <div className="registerForm-item">
          <Field label="Host" name="host" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Port" name="port" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Player Name" name="userName" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Password" name="password" type="password" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Password (again)" name="passwordConfirm" type="password" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Email" name="email" type="email" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Email (again)" name="emailConfirm" type="email" component={InputField} />
        </div>
        <div className="registerForm-item">
          <Field label="Real Name" name="realName" component={InputField} />
        </div>
        <Button className="registerForm-submit" color="primary" variant="contained" type="submit">
          Register
        </Button>
      </Form>
    );
  }
}

const propsMap = {
  form: 'register'
};

const mapStateToProps = () => ({
  initialValues: {
    
  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(RegisterForm));
