// eslint-disable-next-line
import React from "react";
import { connect } from "react-redux";
import { Form, reduxForm } from "redux-form"

import { InputAction } from 'components';

const AddToBuddies = ({ handleSubmit }) => (
  <Form onSubmit={handleSubmit}>
    <InputAction action="Add" label="Add to Buddies" name="userName" />
  </Form>
);

const propsMap = {
  form: "addToBuddies"
};

export default connect()(reduxForm(propsMap)(AddToBuddies));
