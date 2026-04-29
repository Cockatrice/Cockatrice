import { ReactNode } from 'react';
import Dialog, { DialogProps } from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';
import CloseIcon from '@mui/icons-material/Close';

import './AuthDialogShell.css';

export interface AuthDialogShellProps {
  isOpen: boolean;
  handleClose?: () => void;
  title: string;
  children: ReactNode;
  className?: string;
  contentClassName?: string;
  maxWidth?: DialogProps['maxWidth'];
}

const AuthDialogShell = ({
  isOpen,
  handleClose,
  title,
  children,
  className,
  contentClassName,
  maxWidth,
}: AuthDialogShellProps) => {
  const closeGuarded = handleClose ? () => handleClose() : undefined;

  return (
    <Dialog
      className={className}
      onClose={closeGuarded}
      open={isOpen}
      maxWidth={maxWidth}
    >
      <DialogTitle className="dialog-title">
        <Typography variant="h6">{title}</Typography>

        {closeGuarded ? (
          <IconButton onClick={closeGuarded} size="large">
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent className={contentClassName}>
        {children}
      </DialogContent>
    </Dialog>
  );
};

export default AuthDialogShell;
