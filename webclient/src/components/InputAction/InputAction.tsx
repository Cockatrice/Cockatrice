import { Field } from 'react-final-form';
import Button from '@mui/material/Button';

import { InputField } from '..';

import './InputAction.css';

interface InputActionProps {
  action: string;
  label: string;
  name: string;
  validate?: (value: unknown) => string | undefined | false;
  disabled?: boolean;
}

const InputAction = ({
  action,
  label,
  name,
  validate = () => undefined,
  disabled = false,
}: InputActionProps) => (
  <div className="input-action">
    <div className="input-action__item">
      <Field label={label} name={name} component={InputField} validate={validate} />
    </div>
    <div className="input-action__submit">
      <Button color="primary" variant="contained" type="submit" disabled={disabled}>
        {action}
      </Button>
    </div>
  </div>
);

export default InputAction;
