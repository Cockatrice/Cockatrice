import { Form } from 'react-final-form';

import { InputAction } from '@app/components';

interface AddUserFormValues {
  userName: string;
}

interface AddUserFormProps {
  label: string;
  onSubmit: (values: AddUserFormValues) => void;
}

const AddUserForm = ({ label, onSubmit }: AddUserFormProps) => (
  <Form<AddUserFormValues> onSubmit={(values) => onSubmit(values)}>
    {({ handleSubmit }) => (
      <form onSubmit={handleSubmit}>
        <InputAction action="Add" label={label} name="userName" />
      </form>
    )}
  </Form>
);

export default AddUserForm;
