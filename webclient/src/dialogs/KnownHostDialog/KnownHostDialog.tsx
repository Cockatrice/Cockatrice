import React from 'react';
import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import makeStyles from '@mui/styles/makeStyles';
import AddIcon from '@mui/icons-material/Add';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';
import { useTranslation } from 'react-i18next';

import { KnownHostForm } from 'forms';

import './KnownHostDialog.css';

const useStyles = makeStyles(theme => ({
  root: {
    '& .dialog-title__wrapper': {
      borderColor: theme.palette.grey[300]
    }
  },
}));

const KnownHostDialog = ({ handleClose, onRemove, onSubmit, isOpen, host }: any) => {
  const classes = useStyles();
  const { t } = useTranslation();

  const mode = host ? 'edit' : 'add';

  const handleOnClose = () => {
    if (handleClose) {
      handleClose();
    }
  };

  return (
    <Dialog className={'KnownHostDialog ' + classes.root} onClose={handleOnClose} open={isOpen}>
      <DialogTitle className='dialog-title'>
        <div className='dialog-title__wrapper'>
          <Typography variant='h2'>{ t('KnownHostDialog.title', { mode }) }</Typography>

          {handleClose ? (
            <IconButton onClick={handleClose} size="large">
              <CloseIcon fontSize='large' />
            </IconButton>
          ) : null}
        </div>
      </DialogTitle>
      <DialogContent className='dialog-content'>
        <Typography className='dialog-content__subtitle' variant='subtitle1'>
          { t('KnownHostDialog.subtitle') }
        </Typography>
        <KnownHostForm onRemove={onRemove} onSubmit={onSubmit} host={host}></KnownHostForm>
      </DialogContent>
    </Dialog>
  );
};

export default KnownHostDialog;
