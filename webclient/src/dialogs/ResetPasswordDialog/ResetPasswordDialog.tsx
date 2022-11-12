import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import { ResetPasswordForm } from 'forms';

import './ResetPasswordDialog.css';

const ResetPasswordDialog = ({ classes, handleClose, isOpen, onSubmit, userName }: any) => {
  const { t } = useTranslation();

  const handleOnClose = () => {
    handleClose();
  }

  return (
    <Dialog onClose={handleOnClose} open={isOpen}>
      <DialogTitle className="dialog-title">
        <Typography variant="h6">{t('ResetPasswordDialog.title')}</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose} size="large">
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
