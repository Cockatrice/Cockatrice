import React from "react";
import TextField from "@material-ui/core/TextField";

const InputField = ({ input, label, name, autoComplete, type }) => (
  <TextField
    variant="outlined"
    margin="dense"
    fullWidth={true}
    label={label}
    name={name}
    type={type}
    autoComplete={autoComplete}
    { ...input }
  />
);

export default InputField;