import React from "react";
import { makeStyles } from '@material-ui/core/styles';
import TextField from "@material-ui/core/TextField";
import ErrorOutlinedIcon from '@material-ui/icons/ErrorOutlined';

import './InputField.css';

const useStyles = makeStyles(theme => ({
  root: {
    '& .InputField-error': {
      color: theme.palette.error.main
    },

    '& .InputField-warning': {
      color: theme.palette.warning.main
    }
  },
}));

const InputField = ({ input, label, name, autoComplete, type, meta: { touched, error, warning } }) => {
  const classes = useStyles();

  return (
    <div className={"InputField " + classes.root}>
      { touched && (
        <div className="InputField-validation">
          {
            ( error &&
              <div className="InputField-error">
                {error}
                <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
              </div>
            ) ||

            ( warning && <div className="InputField-warning">{warning}</div> )
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
};

export default InputField;
