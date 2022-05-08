import React from 'react';
import { Form } from 'react-final-form'

import { InputAction } from 'components';

const AddToBuddies = ({ onSubmit }) => (
  <Form onSubmit={values => onSubmit(values)}>
    {({ handleSubmit }) => (
      <form onSubmit={handleSubmit}>
        <InputAction action="Add" label="Add to Buddies" name="userName" />
      </form>
    )}
  </Form>
);

export default AddToBuddies;
