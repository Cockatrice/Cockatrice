import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { RegisterForm } from 'forms';

import './RegistrationDialog.css';

const RegistrationDialog = ({ classes, handleClose, isOpen, onSubmit }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog className="RegistrationDialog" onClose={handleOnClose} open={isOpen} maxWidth='xl'>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h6">Create New Account</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent className="dialog-content">
        <RegisterForm onSubmit={onSubmit}></RegisterForm>
      </DialogContent>
    </Dialog>
  );
};

export default RegistrationDialog;
