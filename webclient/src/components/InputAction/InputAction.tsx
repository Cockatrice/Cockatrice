import React from 'react';
import { Field } from 'react-final-form'
import Button from '@mui/material/Button';

import { InputField } from 'components';

import './InputAction.css';

const InputAction = ({ action, label, name, validate, disabled }) => (
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

InputAction.defaultProps = {
  disabled: false,
  validate: () => false,
}

export default InputAction;
