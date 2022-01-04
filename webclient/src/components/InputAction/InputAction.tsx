import React from 'react';
import { Field } from 'react-final-form'
import Button from '@material-ui/core/Button';

import { InputField } from 'components';

import './InputAction.css';

const InputAction = ({ action, label, name, validate, ...rest }) => (
  <div className="input-action">
    <div className="input-action__item">
      <Field label={label} name={name} component={InputField} validate={validate} />
    </div>
    <div className="input-action__submit">
      <Button {...rest} color="primary" variant="contained" type="submit">
        {action}
      </Button>
    </div>
  </div>
);

InputAction.defaultProps = {
  disabled: false,
  validate: () => true,
}

export default InputAction;
