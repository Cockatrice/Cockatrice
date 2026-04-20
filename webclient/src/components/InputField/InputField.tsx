import { styled } from '@mui/material/styles';
import TextField, { TextFieldProps } from '@mui/material/TextField';
import ErrorOutlinedIcon from '@mui/icons-material/ErrorOutlined';

import type { FinalFormFieldProps } from '../fieldTypes';

import './InputField.css';

const PREFIX = 'InputField';

const classes = {
  root: `${PREFIX}-root`,
};

const Root = styled('div')(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .InputField-error': {
      color: theme.palette.error.main,
    },
    '& .InputField-warning': {
      color: theme.palette.warning.main,
    },
  },
}));

type InputFieldProps =
  FinalFormFieldProps<string, HTMLInputElement> &
  Omit<TextFieldProps, 'value' | 'onChange' | 'onBlur' | 'onFocus' | 'name'>;

const InputField = ({ input, meta, ...args }: InputFieldProps) => {
  const { touched, error, warning } = meta;

  return (
    <Root className={`InputField ${classes.root}`}>
      {touched && (
        <div className="InputField-validation">
          {(error &&
            <div className="InputField-error">
              {error}
              <ErrorOutlinedIcon style={{ fontSize: 'small', fontWeight: 'bold' }} />
            </div>
          ) || (warning && <div className="InputField-warning">{warning}</div>)}
        </div>
      )}

      <TextField
        autoComplete="off"
        {...input}
        {...args}
        className="rounded"
        variant="outlined"
        margin="dense"
        size="small"
        fullWidth
      />
    </Root>
  );
};

export default InputField;
