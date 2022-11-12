import React from 'react';
import { Form } from 'react-final-form'

import { InputAction } from 'components';

const SayMessage = ({ onSubmit }) => (
  <Form onSubmit={onSubmit}>
    {({ handleSubmit, form }) => (
      <form onSubmit={e => {
        handleSubmit(e)
        form.restart()
      }}>
        <InputAction action="Send" label="Chat" name="message" />
      </form>
    )}
  </Form>
);

export default SayMessage;
