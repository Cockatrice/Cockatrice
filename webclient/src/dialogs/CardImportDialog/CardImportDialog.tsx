import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

import { CardImportForm } from 'forms';

import './CardImportDialog.css';

const CardImportDialog = ({ classes, handleClose, isOpen }: any) => {
  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h2">Import Cards</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
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
