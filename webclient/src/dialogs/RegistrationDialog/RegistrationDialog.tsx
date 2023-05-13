import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import { RegisterForm } from 'forms';

import './RegistrationDialog.css';

const RegistrationDialog = ({ classes, handleClose, isOpen, onSubmit }: any) => {
  const { t } = useTranslation();

  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog className="RegistrationDialog" onClose={handleOnClose} open={isOpen} maxWidth='xl'>
      <DialogTitle className="dialog-title">
        <Typography variant="h6">{ t('RegistrationDialog.title') }</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose} size="large">
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
