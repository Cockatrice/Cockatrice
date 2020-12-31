// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, reduxForm } from "redux-form"

import InputAction from 'AppShell/common/components/InputAction/InputAction';

const SayMessage = ({ handleSubmit }) => (
  <Form onSubmit={handleSubmit}>
    <InputAction action="Say" label="Chat" name="message" />
  </Form>
);

const propsMap = {
  form: "sayMessage"
};

export default connect()(reduxForm(propsMap)(SayMessage));
