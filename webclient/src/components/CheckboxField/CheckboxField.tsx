import React from 'react';
import Checkbox from '@material-ui/core/Checkbox';
import FormControlLabel from '@material-ui/core/FormControlLabel';

const CheckboxField = (props) => {
  const { input: { value, onChange }, label, ...args } = props;

  // @TODO this isnt unchecking properly
  return (
    <FormControlLabel
      className="checkbox-field"
      label={label}
      control={
        <Checkbox
          { ...args }
          className="checkbox-field__box"
          checked={!!value}
          onChange={(e, checked) => onChange(checked)}
          color="primary"
        />
      }
    />
  );
};

export default CheckboxField;
