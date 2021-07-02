// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, reduxForm } from "redux-form"

import { InputAction } from 'components';
import { FormKey } from 'types';

const AddToIgnore = ({ handleSubmit }) => (
  <Form onSubmit={handleSubmit}>
    <InputAction action="Add" label="Add to Ignore" name="userName" />
  </Form>
);

const propsMap = {
  form: FormKey.ADD_TO_IGNORE,
};

export default connect()(reduxForm(propsMap)(AddToIgnore));
