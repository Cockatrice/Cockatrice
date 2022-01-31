import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { AccountActivationForm } from 'forms';

import './AccountActivationDialog.css';

const AccountActivationDialog = ({ classes, handleClose, isOpen, onSubmit }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h6">Account Activation</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <div className="content">
          <Typography variant='subtitle1'>Your account has not been activated yet.</Typography>
          <Typography variant='subtitle1'>You need to provide the activation token received in the activation email.</Typography>
        </div>

        <AccountActivationForm onSubmit={onSubmit}></AccountActivationForm>
      </DialogContent>
    </Dialog>
  );
};

export default AccountActivationDialog;
