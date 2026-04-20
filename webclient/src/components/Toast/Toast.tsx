import { ReactNode, SyntheticEvent } from 'react';

import Alert, { AlertColor } from '@mui/material/Alert';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import Slide, { SlideProps } from '@mui/material/Slide';
import Snackbar from '@mui/material/Snackbar';

const iconMapping = {
  success: <CheckCircleIcon />,
};

export interface ToastProps {
  open: boolean;
  onClose: (event?: SyntheticEvent) => void;
  severity?: AlertColor;
  autoHideDuration?: number;
  children?: ReactNode;
}

// MUI's Snackbar already self-portals to the end of document.body; adding our
// own createPortal wrapper would leak <div>s under React StrictMode's double-
// invoked effects. Render the Snackbar directly.
function Toast({ open, onClose, severity = 'success', autoHideDuration = 10000, children }: ToastProps) {
  const handleClose = (event?: SyntheticEvent | Event, reason?: string) => {
    if (reason === 'clickaway') {
      return;
    }
    onClose(event as SyntheticEvent | undefined);
  };

  return (
    <Snackbar
      open={open}
      autoHideDuration={autoHideDuration}
      onClose={handleClose}
      slots={{ transition: TransitionLeft }}
      anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
    >
      <Alert
        onClose={handleClose}
        severity={severity}
        iconMapping={iconMapping}
        slotProps={{ message: { children } }}
      />
    </Snackbar>
  );
}

function TransitionLeft(props: SlideProps) {
  return <Slide {...props} direction="left" />;
}

export default Toast;
