import * as React from 'react'
import Snackbar from '@material-ui/core/Snackbar';
import Alert, { AlertProps } from '@material-ui/lab/Alert';
import ReactDOM from 'react-dom'
import CheckCircleIcon from '@material-ui/icons/CheckCircle';

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
    <Snackbar open={open} autoHideDuration={autoHideDuration} onClose={handleClose}>
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
  autoHideDuration: 6000,
}

export default Toast
