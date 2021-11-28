import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { ResetPasswordForm } from 'forms';

import './ResetPasswordDialog.css';

const ResetPasswordDialog = ({ classes, handleClose, isOpen, onSubmit, userName }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h6">Reset Password</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <ResetPasswordForm onSubmit={onSubmit} userName={userName}/>
      </DialogContent>
    </Dialog>
  );
};

export default ResetPasswordDialog;
