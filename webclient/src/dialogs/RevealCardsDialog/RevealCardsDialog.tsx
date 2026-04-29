import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';

import { useRevealCardsDialog } from './useRevealCardsDialog';

import './RevealCardsDialog.css';

const PREFIX = 'RevealCardsDialog';

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

export interface RevealCardsSubmit {
  /** -1 means "all players" (desktop default). */
  targetPlayerId: number;
  /** -1 means "all cards in the zone" (desktop uses -1 for full-hand/full-grave reveals). */
  topCards: number;
}

export interface RevealCardsDialogProps {
  isOpen: boolean;
  title: string;
  zoneLabel: string;
  /** If true, show the "How many?" input; pair with `defaultCount` for the initial value. */
  showCountInput?: boolean;
  defaultCount?: number;
  players: ReadonlyArray<{ playerId: number; name: string }>;
  onSubmit: (args: RevealCardsSubmit) => void;
  onCancel: () => void;
}

const ALL_PLAYERS = -1;

function RevealCardsDialog({
  isOpen,
  title,
  zoneLabel,
  showCountInput = false,
  defaultCount = 1,
  players,
  onSubmit,
  onCancel,
}: RevealCardsDialogProps) {
  const {
    targetPlayerId,
    countDraft,
    error,
    setTargetPlayerId,
    handleCountChange,
    handleSubmit,
  } = useRevealCardsDialog({ isOpen, showCountInput, defaultCount, onSubmit });

  return (
    <StyledDialog
      className={'RevealCardsDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">{title}</Typography>
          <Typography variant="caption" className="reveal-cards-dialog__zone">
            From: {zoneLabel}
          </Typography>
        </div>
      </DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content reveal-cards-dialog__body">
          <FormControl fullWidth size="small" variant="outlined">
            <InputLabel id="reveal-target-label">Reveal to</InputLabel>
            <Select
              labelId="reveal-target-label"
              label="Reveal to"
              value={String(targetPlayerId)}
              onChange={(e) => setTargetPlayerId(Number(e.target.value))}
              slotProps={{ input: { 'aria-label': 'Reveal target' } }}
            >
              <MenuItem value={String(ALL_PLAYERS)}>All players</MenuItem>
              {players.map((p) => (
                <MenuItem key={p.playerId} value={String(p.playerId)}>
                  {p.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          {showCountInput && (
            <TextField
              autoFocus
              fullWidth
              variant="outlined"
              size="small"
              type="number"
              label="How many?"
              value={countDraft}
              onChange={(e) => handleCountChange(e.target.value)}
              onFocus={(e) => e.currentTarget.select()}
              error={error != null}
              helperText={error ?? 'Enter a positive integer'}
              slotProps={{ htmlInput: { 'aria-label': 'Reveal count', min: 1 } }}
            />
          )}
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Reveal</Button>
        </DialogActions>
      </form>
    </StyledDialog>
  );
}

export default RevealCardsDialog;
