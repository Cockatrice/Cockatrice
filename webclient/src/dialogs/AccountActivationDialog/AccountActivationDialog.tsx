import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';
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
      <DialogTitle disableTypography className="dialog-title">
        <Typography variant="h6">{ t('AccountActivationDialog.title') }</Typography>

        {handleOnClose ? (
          <IconButton onClick={handleOnClose}>
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
