import React from "react";
import FormControl from "@material-ui/core/FormControl";
import InputLabel from "@material-ui/core/InputLabel";
import MenuItem from "@material-ui/core/MenuItem";
import Select from "@material-ui/core/Select";

import './SelectField.css';

const SelectField = ({ input, label, options, value }) => {
  const id = label + "-select-field";
  const labelId = id + "-label";

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