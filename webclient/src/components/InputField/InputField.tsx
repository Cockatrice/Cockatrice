import React from 'react';
import makeStyles from '@mui/styles/makeStyles';
import TextField from '@mui/material/TextField';
import ErrorOutlinedIcon from '@mui/icons-material/ErrorOutlined';

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

const InputField = ({ input, meta: { touched, error, warning }, ...args }) => {
  const classes = useStyles();

  return (
    <div className={'InputField ' + classes.root}>
      { touched && (
        <div className="InputField-validation">
          {
            (error &&
              <div className="InputField-error">
                {error}
                <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
              </div>
            ) ||

            (warning && <div className="InputField-warning">{warning}</div>)
          }
        </div>
      ) }

      <TextField
        autoComplete='off'
        { ...input }
        { ...args }
        className="rounded"
        variant="outlined"
        margin="dense"
        size="small"
        fullWidth={true}
      />
    </div>
  );
};

export default InputField;
