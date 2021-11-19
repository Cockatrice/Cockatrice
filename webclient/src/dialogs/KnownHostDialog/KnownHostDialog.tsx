import React from 'react';
import Dialog from '@material-ui/core/Dialog';
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import IconButton from '@material-ui/core/IconButton';
import { makeStyles } from '@material-ui/core/styles';
import AddIcon from '@material-ui/icons/Add';
import CloseIcon from '@material-ui/icons/Close';
import Typography from '@material-ui/core/Typography';

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

  const handleOnClose = () => {
    if (handleClose) {
      handleClose();
    }
  };

  return (
    <Dialog className={'KnownHostDialog ' + classes.root} onClose={handleOnClose} open={isOpen}>
      <DialogTitle disableTypography className='dialog-title'>
        <div className='dialog-title__wrapper'>
          <Typography variant='h2'>{ host ? 'Edit' : 'Add' } Known Host</Typography>

          {handleClose ? (
            <IconButton onClick={handleClose}>
              <CloseIcon fontSize='large' />
            </IconButton>
          ) : null}
        </div>
      </DialogTitle>
      <DialogContent className='dialog-content'>
        <Typography className='dialog-content__subtitle' variant='subtitle1'>
          Adding a new host allows you to connect to different servers. Enter the details below to your host list.
        </Typography>
        <KnownHostForm onRemove={onRemove} onSubmit={onSubmit} host={host}></KnownHostForm>
      </DialogContent>
    </Dialog>
  );
};

export default KnownHostDialog;
