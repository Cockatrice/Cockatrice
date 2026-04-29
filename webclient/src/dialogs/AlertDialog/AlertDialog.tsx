import { styled } from '@mui/material/styles';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogContentText from '@mui/material/DialogContentText';
import DialogTitle from '@mui/material/DialogTitle';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';

import './AlertDialog.css';

const PREFIX = 'AlertDialog';

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

export type AlertDialogSeverity = 'error' | 'info';

export interface AlertDialogProps {
  isOpen: boolean;
  title: string;
  message: string;
  buttonLabel?: string;
  severity?: AlertDialogSeverity;
  onDismiss: () => void;
}

/**
 * Single-button modal alert. Mirrors desktop's QMessageBox::critical pattern
 * (see cockatrice/src/interface/widgets/server/game_selector.cpp:234-260 for
 * the join-game error dialogs this was originally built for).
 */
function AlertDialog({
  isOpen,
  title,
  message,
  buttonLabel = 'OK',
  severity = 'error',
  onDismiss,
}: AlertDialogProps) {
  return (
    <StyledDialog
      className={'AlertDialog ' + classes.root}
      open={isOpen}
      onClose={onDismiss}
      maxWidth={false}
    >
      <DialogTitle className="dialog-title">
        <div className="dialog-title__wrapper">
          <Typography variant="h2">{title}</Typography>
        </div>
      </DialogTitle>
      <DialogContent className="dialog-content alert-dialog__body">
        <DialogContentText>{message}</DialogContentText>
      </DialogContent>
      <DialogActions>
        <Button
          type="button"
          variant="contained"
          color={severity === 'error' ? 'error' : 'primary'}
          onClick={onDismiss}
          autoFocus
        >
          {buttonLabel}
        </Button>
      </DialogActions>
    </StyledDialog>
  );
}

export default AlertDialog;
