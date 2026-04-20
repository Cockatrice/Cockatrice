import * as React from 'react'
import { createPortal } from 'react-dom'

import Alert from '@mui/material/Alert';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import Slide from '@mui/material/Slide';
import Snackbar from '@mui/material/Snackbar';

const iconMapping = {
  success: <CheckCircleIcon />
}

function Toast(props) {
  const { open, onClose, severity = 'success', autoHideDuration = 10000, children } = props

  const rootElemRef = React.useRef(document.createElement('div'));

  React.useEffect(() => {
    document.body.appendChild(rootElemRef.current)
    return () => {
      rootElemRef.current.remove();
    }
  }, [rootElemRef])

  const handleClose = (event?: React.SyntheticEvent, reason?: string) => {
    if (reason === 'clickaway') {
      return;
    }
    onClose(event);
  };

  const node = (
    <Snackbar
      open={open}
      autoHideDuration={autoHideDuration}
      onClose={handleClose}
      slots={{ transition: TransitionLeft }}
      anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
    >
      <Alert onClose={handleClose} severity={severity} iconMapping={iconMapping}
        slotProps={{ message: { children } }}
      />
    </Snackbar>
  )
  if (!rootElemRef.current) {
    return null
  }

  return createPortal(
    node,
    rootElemRef.current
  );
}

function TransitionLeft(props) {
  return <Slide {...props} direction="left" />;
}

export default Toast
