// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Field, InjectedFormProps, reduxForm } from 'redux-form'
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';

import './SayMessage.css';

class SayMessage extends Component<InjectedFormProps> {
  render() {
    return (
      <form className="sayMessage" onSubmit={this.props.handleSubmit}>
        <div className="sayMessage-item">
          <Field
            label="Chat"
            name="message"
            component={InputField}
          />
        </div>
        <div className="sayMessage-submit">
          <Button color="primary" variant="contained" type="submit">
            Say
          </Button>
        </div>
      </form>
    );
  }
}

const InputField = ({
  input,
  label,
  name,
  meta: { touched, error },
  className
}) => (
  <TextField
    label={label}
    name={name}
    fullWidth={true}
    margin="dense"
    variant="outlined"
    { ...input }
  />
);

const propsMap = {
  form: 'sayMessage'
};

export default connect()(reduxForm(propsMap)(SayMessage));
