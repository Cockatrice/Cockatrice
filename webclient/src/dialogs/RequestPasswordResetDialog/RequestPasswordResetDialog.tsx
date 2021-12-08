import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { RequestPasswordResetForm } from 'forms';

import './RequestPasswordResetDialog.css';

const RequestPasswordResetDialog = ({ classes, handleClose, isOpen, onSubmit, skipTokenRequest }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h6">Request Password Reset</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <RequestPasswordResetForm onSubmit={onSubmit} skipTokenRequest={skipTokenRequest}></RequestPasswordResetForm>
      </DialogContent>
    </Dialog>
  );
};

export default RequestPasswordResetDialog;
