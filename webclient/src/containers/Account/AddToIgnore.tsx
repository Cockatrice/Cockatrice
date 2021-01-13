// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, reduxForm } from "redux-form"

import { InputAction } from 'components';

const AddToIgnore = ({ handleSubmit }) => (
  <Form onSubmit={handleSubmit}>
    <InputAction action="Add" label="Add to Ignore" name="userName" />
  </Form>
);

const propsMap = {
  form: "addToIgnore"
};

export default connect()(reduxForm(propsMap)(AddToIgnore));
