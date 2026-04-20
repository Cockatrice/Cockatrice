import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import FormControlLabel from '@mui/material/FormControlLabel';
import Checkbox from '@mui/material/Checkbox';
import Select from '@mui/material/Select';
import MenuItem from '@mui/material/MenuItem';
import InputLabel from '@mui/material/InputLabel';
import FormControl from '@mui/material/FormControl';

import {
  MAX_ANNOTATION_LEN,
  MAX_NAME_LEN,
  MAX_PT_LEN,
  useCreateTokenDialog,
} from './useCreateTokenDialog';

import './CreateTokenDialog.css';

const PREFIX = 'CreateTokenDialog';

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

export interface CreateTokenSubmit {
  name: string;
  color: string;
  pt: string;
  annotation: string;
  destroyOnZoneChange: boolean;
  faceDown: boolean;
}

export interface CreateTokenDialogProps {
  isOpen: boolean;
  onSubmit: (args: CreateTokenSubmit) => void;
  onCancel: () => void;
}

// Matches desktop DlgCreateToken color dropdown values. Desktop orders
// White → Blue → Black → Red → Green → Multicolor → Colorless and defaults
// to White; we mirror both.
const COLOR_OPTIONS: ReadonlyArray<{ value: string; label: string }> = [
  { value: 'w', label: 'White' },
  { value: 'u', label: 'Blue' },
  { value: 'b', label: 'Black' },
  { value: 'r', label: 'Red' },
  { value: 'g', label: 'Green' },
  { value: 'm', label: 'Multicolor' },
  { value: '', label: 'Colorless' },
];

function CreateTokenDialog({ isOpen, onSubmit, onCancel }: CreateTokenDialogProps) {
  const {
    name,
    color,
    pt,
    annotation,
    destroyOnZoneChange,
    faceDown,
    error,
    handleNameChange,
    setColor,
    setPT,
    setAnnotation,
    setDestroyOnZoneChange,
    setFaceDown,
    handleSubmit,
  } = useCreateTokenDialog({ isOpen, onSubmit });

  return (
    <StyledDialog
      className={'CreateTokenDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">Create token</Typography>
        </div>
      </DialogTitle>
      <form onSubmit={handleSubmit}>
        <DialogContent className="dialog-content create-token-dialog__body">
          <TextField
            autoFocus
            fullWidth
            variant="outlined"
            size="small"
            label="Token name"
            value={name}
            onChange={(e) => handleNameChange(e.target.value)}
            error={error != null}
            helperText={error ?? ''}
            slotProps={{ htmlInput: { 'aria-label': 'Token name', maxLength: MAX_NAME_LEN } }}
          />
          <FormControl fullWidth size="small" variant="outlined" disabled={faceDown}>
            <InputLabel id="create-token-color-label">Color</InputLabel>
            <Select
              labelId="create-token-color-label"
              label="Color"
              value={color}
              onChange={(e) => setColor(e.target.value)}
              slotProps={{ input: { 'aria-label': 'Token color' } }}
            >
              {COLOR_OPTIONS.map((opt) => (
                <MenuItem key={opt.label} value={opt.value}>
                  {opt.label}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          <TextField
            fullWidth
            variant="outlined"
            size="small"
            label="Token power/toughness"
            placeholder="e.g. 3/3"
            value={pt}
            onChange={(e) => setPT(e.target.value.slice(0, MAX_PT_LEN))}
            disabled={faceDown}
            slotProps={{ htmlInput: { 'aria-label': 'Token power/toughness', maxLength: MAX_PT_LEN } }}
          />
          <TextField
            fullWidth
            variant="outlined"
            size="small"
            label="Token annotation"
            value={annotation}
            onChange={(e) => setAnnotation(e.target.value.slice(0, MAX_ANNOTATION_LEN))}
            slotProps={{ htmlInput: { 'aria-label': 'Token annotation', maxLength: MAX_ANNOTATION_LEN } }}
          />
          <FormControlLabel
            control={
              <Checkbox
                checked={destroyOnZoneChange}
                onChange={(e) => setDestroyOnZoneChange(e.target.checked)}
                slotProps={{ input: { 'aria-label': 'Destroy when it leaves the table' } }}
              />
            }
            label="Destroy when it leaves the table"
          />
          <FormControlLabel
            control={
              <Checkbox
                checked={faceDown}
                onChange={(e) => setFaceDown(e.target.checked)}
                slotProps={{ input: { 'aria-label': 'Create face-down' } }}
              />
            }
            label="Create face-down"
          />
        </DialogContent>
        <DialogActions>
          <Button type="button" onClick={onCancel}>Cancel</Button>
          <Button type="submit" variant="contained" color="primary">Create</Button>
        </DialogActions>
      </form>
    </StyledDialog>
  );
}

export default CreateTokenDialog;
