import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';

import { App } from '@app/types';
import { cx } from '@app/utils';

import { useCreateCounterDialog } from './useCreateCounterDialog';

import './CreateCounterDialog.css';

const PREFIX = 'CreateCounterDialog';

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

export interface CounterColor {
  r: number;
  g: number;
  b: number;
  a: number;
}

export interface CreateCounterDialogProps {
  isOpen: boolean;
  onSubmit: (args: { name: string; color: CounterColor }) => void;
  onCancel: () => void;
}

interface Swatch {
  label: string;
  color: CounterColor;
  css: string;
}

const SWATCHES: ReadonlyArray<Swatch> = [
  { label: 'White', color: { r: 249, g: 248, b: 217, a: 255 }, css: '#f9f8d9' },
  { label: 'Blue', color: App.ArrowColor.BLUE, css: '#89b8e0' },
  { label: 'Black', color: { r: 60, g: 60, b: 60, a: 255 }, css: '#3c3c3c' },
  { label: 'Red', color: App.ArrowColor.RED, css: '#e04b3b' },
  { label: 'Green', color: App.ArrowColor.GREEN, css: '#3da26b' },
  { label: 'Yellow', color: App.ArrowColor.YELLOW, css: '#f0c83c' },
  { label: 'Purple', color: { r: 148, g: 90, b: 200, a: 255 }, css: '#945ac8' },
  { label: 'Gray', color: { r: 160, g: 160, b: 168, a: 255 }, css: '#a0a0a8' },
];

function CreateCounterDialog({ isOpen, onSubmit, onCancel }: CreateCounterDialogProps) {
  const { name, selectedIdx, error, handleNameChange, setSelectedIdx, handleSubmit } =
    useCreateCounterDialog({ isOpen, swatches: SWATCHES, onSubmit });

  return (
    <StyledDialog
      className={'CreateCounterDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">New counter</Typography>
        </div>
      </DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content">
          <TextField
            autoFocus
            fullWidth
            variant="outlined"
            size="small"
            label="Counter name"
            value={name}
            onChange={(e) => handleNameChange(e.target.value)}
            error={error != null}
            helperText={error ?? ''}
            slotProps={{ htmlInput: { 'aria-label': 'Counter name' } }}
          />
          <div
            className="create-counter-dialog__swatches"
            role="radiogroup"
            aria-label="Counter color"
            onKeyDown={(e) => {
              if (e.key === 'ArrowRight' || e.key === 'ArrowDown') {
                e.preventDefault();
                setSelectedIdx((selectedIdx + 1) % SWATCHES.length);
              } else if (e.key === 'ArrowLeft' || e.key === 'ArrowUp') {
                e.preventDefault();
                setSelectedIdx((selectedIdx - 1 + SWATCHES.length) % SWATCHES.length);
              }
            }}
          >
            {SWATCHES.map((s, idx) => (
              <button
                key={s.label}
                type="button"
                role="radio"
                aria-checked={idx === selectedIdx}
                aria-label={s.label}
                tabIndex={idx === selectedIdx ? 0 : -1}
                className={cx('create-counter-dialog__swatch', {
                  'create-counter-dialog__swatch--selected': idx === selectedIdx,
                })}
                style={{ background: s.css }}
                onClick={() => setSelectedIdx(idx)}
              />
            ))}
          </div>
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Create</Button>
        </DialogActions>
      </form>
    </StyledDialog>
  );
}

export default CreateCounterDialog;
