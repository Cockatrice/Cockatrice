import { Suspense, useEffect } from 'react';
import { Provider } from 'react-redux';
import { MemoryRouter as Router } from 'react-router-dom';
import CssBaseline from '@mui/material/CssBaseline';
import { store } from '@app/store';
import Routes from './AppShellRoutes';
import FeatureDetection from './FeatureDetection';

import './AppShell.css';

import { ToastProvider } from '@app/components'

function AppShell() {
  useEffect(() => {
    // @TODO (1)
    window.onbeforeunload = () => true;
  }, []);

  const handleContextMenu = (event) => {
    event.preventDefault();
  };

  return (
    <Suspense fallback="loading">
      <Provider store={store}>
        <CssBaseline />
        <ToastProvider>
          <div className="AppShell" onContextMenu={handleContextMenu}>
            <Router>
              <FeatureDetection />
              <Routes />
            </Router>
          </div>
        </ToastProvider>
      </Provider>
    </Suspense>
  );
}

export default AppShell;
