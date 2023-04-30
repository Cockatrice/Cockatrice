import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import { RequestPasswordResetForm } from 'forms';

import './RequestPasswordResetDialog.css';

const RequestPasswordResetDialog = ({ classes, handleClose, isOpen, onSubmit, skipTokenRequest }: any) => {
  const { t } = useTranslation();

  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle className="dialog-title">
        <Typography variant="h6">{ t('RequestPasswordResetDialog.title') }</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose} size="large">
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
