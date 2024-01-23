import React from 'react';
import FormControl from '@mui/material/FormControl';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';

import './SelectField.css';

const SelectField = ({ input, label, options, value }) => {
  const id = label + '-select-field';
  const labelId = id + '-label';

  return (
    <FormControl variant="outlined" margin="dense" className="select-field">
      <InputLabel id={labelId}>{label}</InputLabel>
      <Select
        labelId={labelId}
        id={id}
        value={value}
        { ...input }
      >{
          options.map((option, index) => (
            <MenuItem value={index} key={index}> { option } </MenuItem>
          ))
        }</Select>
    </FormControl>
  );
};

export default SelectField;
