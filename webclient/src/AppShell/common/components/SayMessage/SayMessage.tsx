// eslint-disable-next-line
import React, { Component } from 'react';
import { connect } from 'react-redux';
import { Field, InjectedFormProps, reduxForm } from 'redux-form'

import './SayMessage.css';

class SayMessage extends Component<InjectedFormProps> {
  render() {
    return (
      <form className="sayMessage" onSubmit={this.props.handleSubmit}>
        <div className="sayMessage-item">
          <Field className="sayMessage-item__field"
            name="message"
            component="input"
            type="text"
          />
        </div>
        <button className="sayMessage-submit" type="submit">>></button>
      </form>
    );
  }
}

const propsMap = {
  form: 'sayMessage'
};

export default connect()(reduxForm(propsMap)(SayMessage));
