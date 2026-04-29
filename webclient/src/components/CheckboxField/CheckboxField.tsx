import Checkbox, { CheckboxProps } from '@mui/material/Checkbox';
import FormControlLabel from '@mui/material/FormControlLabel';

import type { FinalFormFieldProps } from '../fieldTypes';

type CheckboxFieldProps = FinalFormFieldProps<boolean, HTMLInputElement> & {
  label?: string;
} & Omit<CheckboxProps, 'checked' | 'onChange' | 'onBlur' | 'onFocus' | 'name' | 'value'>;

const CheckboxField = ({ input, meta: _meta, label, ...args }: CheckboxFieldProps) => {
  const { value, onChange, onBlur, onFocus, name } = input;

  return (
    <FormControlLabel
      className="checkbox-field"
      label={label ?? ''}
      control={
        <Checkbox
          {...args}
          className="checkbox-field__box"
          name={name}
          checked={Boolean(value)}
          onChange={onChange}
          onBlur={onBlur}
          onFocus={onFocus}
          color="primary"
        />
      }
    />
  );
};

export default CheckboxField;
