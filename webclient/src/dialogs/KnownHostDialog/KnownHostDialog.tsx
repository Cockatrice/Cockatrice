import React from "react";
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { KnownHostForm } from 'forms';

import './KnownHostDialog.css';

const KnownHostDialog = ({ classes, handleClose, onSubmit, isOpen, host }: any) => {
  const handleOnClose = () => {
    if (handleClose) {
      handleClose();
    }
  };

  return (
    <Dialog className='KnownHostDialog' onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h2">{ host ? 'Edit' : 'Add' } Known Host</Typography>

        {handleClose ? (
          <IconButton onClick={handleClose}>
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <Typography variant='subtitle1'>
          Adding a new host allows you to connect to different servers. Enter the details below to your host list.
        </Typography>
        <KnownHostForm onSubmit={onSubmit} host={host}></KnownHostForm>
      </DialogContent>
    </Dialog>
  );
};

export default KnownHostDialog;
