import Dialog from '@mui/material/Dialog';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import IconButton from '@mui/material/IconButton';
import CloseIcon from '@mui/icons-material/Close';
import Typography from '@mui/material/Typography';

import { CardImportForm } from '@app/forms';

import './CardImportDialog.css';

export interface CardImportDialogProps {
  isOpen: boolean;
  handleClose: () => void;
}

const CardImportDialog = ({ handleClose, isOpen }: CardImportDialogProps) => {
  return (
    <Dialog onClose={handleClose} open={isOpen}>
      <DialogTitle className="dialog-title">
        <Typography variant="h2">Import Cards</Typography>

        <IconButton onClick={handleClose} size="large">
          <CloseIcon />
        </IconButton>
      </DialogTitle>
      <DialogContent>
        <CardImportForm onSubmit={handleClose} />
      </DialogContent>
    </Dialog>
  );
};

export default CardImportDialog;
