import * as React from 'react'
import ReactDOM from 'react-dom'

import Alert, { AlertProps } from '@mui/material/Alert';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';
import Slide, { SlideProps } from '@mui/material/Slide';
import Snackbar from '@mui/material/Snackbar';

const iconMapping = {
  success: <CheckCircleIcon />
}

function Toast(props) {
  const { open, onClose, severity, autoHideDuration, children } = props

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
      TransitionComponent={TransitionLeft}
      anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
    >
      <Alert onClose={handleClose} severity={severity} iconMapping={iconMapping}>
        {children}
      </Alert>
    </Snackbar>
  )
  if (!rootElemRef.current) {
    return null
  }

  return ReactDOM.createPortal(
    node,
    rootElemRef.current
  );
}

Toast.defaultProps = {
  severity: 'success',
  // 10s wait before automatically dismissing the Toast.
  autoHideDuration: 10000,
}

function TransitionLeft(props) {
  return <Slide {...props} direction="left" />;
}

export default Toast
