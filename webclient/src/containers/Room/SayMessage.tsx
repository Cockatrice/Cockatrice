import React from 'react';
import { Form } from 'react-final-form'

import { InputAction } from 'components';

const required = (value) => (value ? undefined : 'Required');

const SayMessage = (props) => {
  const { onSubmit, disableSendButton } = props
  return (
    <Form onSubmit={values => onSubmit(values)}>
      {({ handleSubmit }) => (
        <form onSubmit={handleSubmit}>
          <InputAction action="Send" label="Chat" name="message" disabled={disableSendButton} validate={required}/>
        </form>
      )}
    </Form>
  );
}

export default SayMessage;
