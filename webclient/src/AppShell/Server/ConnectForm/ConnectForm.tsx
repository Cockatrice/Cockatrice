// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Field, InjectedFormProps, reduxForm } from 'redux-form'

import Button from '@material-ui/core/Button';
import Paper from '@material-ui/core/Paper';

import InputField from 'AppShell/common/components/InputField/InputField';

import './ConnectForm.css';

class ConnectForm extends Component<InjectedFormProps> {
  render() {
    return (
      <Paper>
        <form className="connectForm" onSubmit={this.props.handleSubmit}>
          <div className="connectForm-item">
            <Field label="Host" name="host" component={InputField} />
          </div>
          <div className="connectForm-item">
            <Field label="Port" name="port" component={InputField} />
          </div>
          <div className="connectForm-item">
            <Field label="User" name="user" component={InputField} autoComplete="username" />
          </div>
          <div className="connectForm-item">
            <Field label="Pass" name="pass" type="password" component={InputField} autoComplete="current-password" />
          </div>
          <Button className="connectForm-submit" color="primary" variant="contained" type="submit">
            Connect
          </Button>
        </form>
      </Paper>
    );
  }
}

const propsMap = {
  form: 'connect'
};

const mapStateToProps = () => ({
  initialValues: {
    host: 'mtg.tetrarch.co/servatrice',
    port: '443'
  }
});

export default connect(mapStateToProps)(reduxForm(propsMap)(ConnectForm));
