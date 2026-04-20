import { useEffect, useState } from 'react';
import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';

import './RollDieDialog.css';

const PREFIX = 'RollDieDialog';

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

/** Shared defaults; re-exported for Game.tsx's persisted "last die" state. */
export const DEFAULT_DIE_SIDES = 6;
export const DEFAULT_DIE_COUNT = 1;

export interface RollDieDialogProps {
  isOpen: boolean;
  lastSides?: number;
  lastCount?: number;
  onSubmit: (args: { sides: number; count: number }) => void;
  onCancel: () => void;
}

function RollDieDialog({
  isOpen,
  lastSides = DEFAULT_DIE_SIDES,
  lastCount = DEFAULT_DIE_COUNT,
  onSubmit,
  onCancel,
}: RollDieDialogProps) {
  const [sides, setSides] = useState(String(lastSides));
  const [count, setCount] = useState(String(lastCount));
  const [error, setError] = useState<{ field: 'sides' | 'count'; message: string } | null>(null);

  useEffect(() => {
    if (isOpen) {
      setSides(String(lastSides));
      setCount(String(lastCount));
      setError(null);
    }
  }, [isOpen, lastSides, lastCount]);

  const handleSubmit = (e?: React.FormEvent<HTMLFormElement>) => {
    e?.preventDefault();
    const s = Number(sides);
    if (!Number.isInteger(s) || s < 1) {
      setError({ field: 'sides', message: 'Enter an integer ≥ 1' });
      return;
    }
    const c = Number(count);
    if (!Number.isInteger(c) || c < 1) {
      setError({ field: 'count', message: 'Enter an integer ≥ 1' });
      return;
    }
    onSubmit({ sides: s, count: c });
  };

  return (
    <StyledDialog
      className={'RollDieDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">Roll die</Typography>
        </div>
      </DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content">
          <TextField
            autoFocus
            fullWidth
            variant="outlined"
            size="small"
            label="Sides"
            value={sides}
            onChange={(e) => {
              setSides(e.target.value);
              if (error) {
                setError(null);
              }
            }}
            error={error?.field === 'sides'}
            helperText={error?.field === 'sides' ? error.message : ''}
            slotProps={{ htmlInput: { 'aria-label': 'Sides', inputMode: 'numeric' } }}
          />
          <TextField
            fullWidth
            variant="outlined"
            size="small"
            label="Count"
            value={count}
            onChange={(e) => {
              setCount(e.target.value);
              if (error) {
                setError(null);
              }
            }}
            error={error?.field === 'count'}
            helperText={error?.field === 'count' ? error.message : ''}
            slotProps={{ htmlInput: { 'aria-label': 'Count', inputMode: 'numeric' } }}
            sx={{ marginTop: '12px' }}
          />
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Roll</Button>
        </DialogActions>
      </form>
    </StyledDialog>
  );
}

export default RollDieDialog;
