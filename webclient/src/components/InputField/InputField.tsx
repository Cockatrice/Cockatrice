import React from 'react';
import { styled } from '@material-ui/core/styles';
import TextField from '@material-ui/core/TextField';
import ErrorOutlinedIcon from '@material-ui/icons/ErrorOutlined';

import './InputField.css';

const InputField = ({ input, label, name, autoComplete, type, meta: { touched, error, warning } }) => (
  <div className="inputField">
    { touched && (
      <div className="inputField-validation">
        {
          (error &&
            <ThemedFieldError className="inputField-error">
              {error}
              <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
            </ThemedFieldError>
          ) ||

          (warning && <ThemedFieldWarning className="inputField-warning">{warning}</ThemedFieldWarning>)
        }
      </div>
    ) }

    <TextField
      className="rounded"
      variant="outlined"
      margin="dense"
      fullWidth={true}
      label={label}
      name={name}
      type={type}
      autoComplete={autoComplete}
      { ...input }
    />
  </div>
);

const ThemedFieldError = styled('div')(({ theme }) => ({
  color: theme.palette.error.main
}));

const ThemedFieldWarning = styled('div')(({ theme }) => ({
  color: theme.palette.warning.main
}));

export default InputField;
