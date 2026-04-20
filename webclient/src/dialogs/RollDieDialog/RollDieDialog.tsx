import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';

import { useRollDieDialog } from './useRollDieDialog';

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
  const { sides, count, error, handleSidesChange, handleCountChange, handleSubmit } =
    useRollDieDialog({ isOpen, lastSides, lastCount, onSubmit });

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
            onChange={(e) => handleSidesChange(e.target.value)}
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
            onChange={(e) => handleCountChange(e.target.value)}
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
