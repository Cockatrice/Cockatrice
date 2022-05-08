import React from 'react';
import { Form } from 'react-final-form'

import { InputAction } from 'components';

const AddToIgnore = ({ onSubmit }) => (
  <Form onSubmit={values => onSubmit(values)}>
    {({ handleSubmit }) => (
      <form onSubmit={handleSubmit}>
        <InputAction action="Add" label="Add to Ignore" name="userName" />
      </form>
    )}
  </Form>
);

export default AddToIgnore;
