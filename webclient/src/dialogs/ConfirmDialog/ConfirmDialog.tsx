import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogContentText from '@mui/material/DialogContentText';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';

import './ConfirmDialog.css';

const PREFIX = 'ConfirmDialog';

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

export interface ConfirmDialogProps {
  isOpen: boolean;
  title: string;
  message: string;
  confirmLabel?: string;
  cancelLabel?: string;
  /** Marks the confirm button as destructive (red). */
  destructive?: boolean;
  onConfirm: () => void;
  onCancel: () => void;
}

/**
 * Generic confirm-before-action dialog. Mirrors desktop's QMessageBox
 * question pattern used for destructive actions (concede, kick, etc.
 * see cockatrice/src/interface/widgets/tabs/tab_game.cpp:487-496).
 */
function ConfirmDialog({
  isOpen,
  title,
  message,
  confirmLabel = 'Confirm',
  cancelLabel = 'Cancel',
  destructive = false,
  onConfirm,
  onCancel,
}: ConfirmDialogProps) {
  return (
    <StyledDialog
      className={'ConfirmDialog ' + classes.root}
      open={isOpen}
      onClose={onCancel}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">{title}</Typography>
        </div>
      </DialogTitle>
      <DialogContent className="dialog-content confirm-dialog__body">
        <DialogContentText>{message}</DialogContentText>
      </DialogContent>
      <DialogActions>
        <Button type="button" onClick={onCancel}>{cancelLabel}</Button>
        <Button
          type="button"
          variant="contained"
          color={destructive ? 'error' : 'primary'}
          onClick={onConfirm}
          autoFocus
        >
          {confirmLabel}
        </Button>
      </DialogActions>
    </StyledDialog>
  );
}

export default ConfirmDialog;
