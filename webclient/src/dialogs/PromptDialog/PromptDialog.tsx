import { useEffect, useState } from 'react';
import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';

import './PromptDialog.css';

const PREFIX = 'PromptDialog';

const classes = {
  root: `${PREFIX}-root`,
};

const StyledDialog = styled(Dialog)(({ theme }) => ({
  [`&.${classes.root}`]: {
    '& .dialog-title__wrapper': {
      borderColor: theme.palette.grey[300],
    },
  },
}));

export interface PromptDialogProps {
  isOpen: boolean;
  title: string;
  label: string;
  initialValue?: string;
  submitLabel?: string;
  /** Persistent helper text shown when no validation error is active. */
  helperText?: string;
  validate?: (value: string) => string | null;
  onSubmit: (value: string) => void;
  onCancel: () => void;
}

function PromptDialog({
  isOpen,
  title,
  label,
  initialValue = '',
  submitLabel = 'OK',
  helperText,
  validate,
  onSubmit,
  onCancel,
}: PromptDialogProps) {
  const [value, setValue] = useState(initialValue);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setValue(initialValue);
      setError(null);
    }
  }, [isOpen, initialValue]);

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    if (validate) {
      const message = validate(value);
      if (message) {
        setError(message);
        return;
      }
    }
    onSubmit(value);
  };

  return (
    <StyledDialog
      className={'PromptDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">{title}</Typography>
        </div>
      </DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content">
          <TextField
            autoFocus
            fullWidth
            variant="outlined"
            size="small"
            label={label}
            value={value}
            onChange={(e) => {
              setValue(e.target.value);
              if (error) {
                setError(null);
              }
            }}
            error={error != null}
            helperText={error ?? helperText ?? ''}
            slotProps={{ htmlInput: { 'aria-label': label } }}
          />
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>
            Cancel
          </Button>
          <Button type="submit" variant="contained" color="primary">
            {submitLabel}
          </Button>
        </DialogActions>
      </form>
    </StyledDialog>
  );
}

export default PromptDialog;
