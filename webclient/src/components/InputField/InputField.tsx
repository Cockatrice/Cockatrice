import React from 'react';
import { styled } from '@mui/material/styles';
import TextField from '@mui/material/TextField';
import ErrorOutlinedIcon from '@mui/icons-material/ErrorOutlined';

import './InputField.css';

const PREFIX = 'InputField';

const classes = {
  root: `${PREFIX}-root`
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .InputField-error': {
      color: theme.palette.error.main
    },

    '& .InputField-warning': {
      color: theme.palette.warning.main
    },
  },
}));

const InputField = ({ input, meta, ...args }) => {
  const { touched, error, warning } = meta;

  return (
    <Root className={'InputField ' + classes.root}>
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
    </Root>
  );
};

export default InputField;
