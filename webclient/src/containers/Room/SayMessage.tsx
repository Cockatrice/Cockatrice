import { Form } from 'react-final-form';

import { InputAction } from '@app/components';

interface SayMessageProps {
  onSubmit: (args: { message: string }) => void;
}

const SayMessage = ({ onSubmit }: SayMessageProps) => (
  <Form onSubmit={onSubmit}>
    {({ handleSubmit, form }) => (
      <form onSubmit={e => {
        handleSubmit(e);
        form.restart();
      }}>
        <InputAction action="Send" label="Chat" name="message" />
      </form>
    )}
  </Form>
);

export default SayMessage;
