import React from 'react';
import {
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  Button,
  Typography,
  Box,
  Alert
} from '@mui/material';
import { Warning as WarningIcon } from '@mui/icons-material';

interface RefreshGuardModalProps {
  open: boolean;
  onClose: () => void;
  onConfirm: () => void;
  message: string;
  title?: string;
  confirmButtonText?: string;
  cancelButtonText?: string;
}

/**
 * Modal dialog that warns users about navigating away or refreshing the page.
 * Synchronized with browser's native beforeunload dialog.
 */
export const RefreshGuardModal: React.FC<RefreshGuardModalProps> = ({
  open,
  onClose,
  onConfirm,
  message,
  title = 'Warning',
  confirmButtonText = 'Leave Anyway',
  cancelButtonText = 'Stay'
}) => {
  return (
    <Dialog
      open={open}
      onClose={onClose}
      aria-labelledby="refresh-guard-title"
      aria-describedby="refresh-guard-description"
      maxWidth="sm"
      fullWidth
      disableEscapeKeyDown // Prevent closing with Escape key
    >
      <DialogTitle id="refresh-guard-title">
        <Box display="flex" alignItems="center" gap={1}>
          <WarningIcon color="warning" />
          {title}
        </Box>
      </DialogTitle>

      <DialogContent>
        <Alert severity="warning" sx={{ mb: 2 }}>
          <Typography variant="body1" id="refresh-guard-description">
            {message}
          </Typography>
        </Alert>

        <Typography variant="body2" color="textSecondary">
          Your browser may also show a confirmation dialog. Both dialogs are asking the same thing -
          whether you want to leave the page and lose your current progress.
        </Typography>
      </DialogContent>

      <DialogActions>
        <Button
          onClick={onClose}
          variant="contained"
          color="primary"
          autoFocus // Focus the safe option by default
        >
          {cancelButtonText}
        </Button>
        <Button
          onClick={onConfirm}
          variant="outlined"
          color="error"
        >
          {confirmButtonText}
        </Button>
      </DialogActions>
    </Dialog>
  );
};
