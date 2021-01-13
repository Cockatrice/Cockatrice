import React from "react";
import Checkbox from "@material-ui/core/Checkbox";
import FormControlLabel from "@material-ui/core/FormControlLabel";

const CheckboxField = ({ input, label }) => {
  const { value, onChange } = input;

  // @TODO this isnt unchecking properly
  return (
    <FormControlLabel
      className="checkbox-field"
      label={label}
      control={
        <Checkbox
          className="checkbox-field__box"
          checked={!!value}
          onChange={onChange}
          color="primary"
        />
      }
    />
  );
};

export default CheckboxField;
