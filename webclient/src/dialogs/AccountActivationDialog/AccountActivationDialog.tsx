import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import { AccountActivationForm } from 'forms';

import './AccountActivationDialog.css';

const AccountActivationDialog = ({ classes, handleClose, isOpen, onSubmit }: any) => {
  const { t } = useTranslation();

  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle className="dialog-title">
        <Typography variant="h6">{ t('AccountActivationDialog.title') }</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose} size="large">
            <CloseIcon />
          </IconButton>
        ) : null}
      </DialogTitle>
      <DialogContent>
        <div className="content">
          <Typography variant='subtitle1'>{ t('AccountActivationDialog.subtitle1') }</Typography>
          <Typography variant='subtitle1'>{ t('AccountActivationDialog.subtitle2') }</Typography>
        </div>

        <AccountActivationForm onSubmit={onSubmit}></AccountActivationForm>
      </DialogContent>
    </Dialog>
  );
};

export default AccountActivationDialog;
