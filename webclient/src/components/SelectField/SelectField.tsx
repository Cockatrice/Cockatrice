import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';

import type { FinalFormFieldProps } from '../fieldTypes';

import './SelectField.css';

export interface SelectFieldOption<V extends string | number = string | number> {
  value: V;
  label: string;
}

interface SelectFieldProps<V extends string | number = string | number> extends FinalFormFieldProps<V, HTMLElement> {
  label: string;
  options: SelectFieldOption<V>[];
}

const SelectField = <V extends string | number = string | number>({
  input,
  label,
  options,
}: SelectFieldProps<V>) => {
  const id = `${label}-select-field`;
  const labelId = `${id}-label`;

  return (
    <FormControl variant="outlined" margin="dense" className="select-field">
      <InputLabel id={labelId}>{label}</InputLabel>
      <Select
        labelId={labelId}
        id={id}
        label={label}
        {...input}
      >
        {options.map(option => (
          <MenuItem value={option.value} key={option.value}>
            {option.label}
          </MenuItem>
        ))}
      </Select>
    </FormControl>
  );
};

export default SelectField;
