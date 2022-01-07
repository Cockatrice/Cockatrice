import React from 'react';
import { Form } from 'react-final-form'

import { InputAction } from 'components';

const required = (value) => (value ? undefined : 'Required');

const SayMessage = (props) => {
  const { onSubmit } = props
  return (
    <Form onSubmit={values => onSubmit(values)}>
      {({ handleSubmit, form }) => (
        <form onSubmit={e => {
          handleSubmit(e)
          form.restart()
        }}>
          <InputAction action="Send" label="Chat" name="message" validate={required}/>
        </form>
      )}
    </Form>
  );
}

export default SayMessage;
