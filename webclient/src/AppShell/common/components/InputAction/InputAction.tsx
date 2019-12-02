// eslint-disable-next-line
import React from "react";
import { Field } from "redux-form"
import Button from "@material-ui/core/Button";

import InputField from '../InputField/InputField';

import "./InputAction.css";

const InputAction = ({ action, label, name }) => (
  <div className="input-action">
    <div className="input-action__item">
      <Field label={label} name={name} component={InputField} />
    </div>
    <div className="input-action__submit">
      <Button color="primary" variant="contained" type="submit">
        {action}
      </Button>
    </div>
  </div>
);

export default InputAction;
