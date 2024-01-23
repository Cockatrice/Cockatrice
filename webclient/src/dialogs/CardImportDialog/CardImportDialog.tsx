import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';

import { CardImportForm } from 'forms';

import './CardImportDialog.css';

const CardImportDialog = ({ classes, handleClose, isOpen }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle className="dialog-title">
        <Typography variant="h2">Import Cards</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose} size="large">
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <CardImportForm onSubmit={handleOnClose}></CardImportForm>
      </DialogContent>
    </Dialog>
  );
};

export default CardImportDialog;
